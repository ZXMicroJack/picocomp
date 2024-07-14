#include "cvideo.h"
#include "cvideo.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include <stdio.h>

// Sync PIO needs 2us per instruction
#define SYNC_INTERVAL 0.000002
// Data transmits for 52us
#define DATA_INTERVAL 0.000052

#define DATA_SM_ID 0
#define SYNC_SM_ID 1

static PIO cvideo_pio;
static uint pio_irq_id = PIO0_IRQ_1;
static cvideo_data_callback_t data_callback;

static inline void cvsync_program_init(PIO pio, uint sm, uint offset, float clockdiv, uint sync_pin);
static inline void cvdata_program_init(PIO pio, uint sm, uint offset, float clockdiv, uint data_pin); 

static void cvdata_isr(void) {
    pio_sm_put(cvideo_pio, DATA_SM_ID, data_callback());
    irq_clear(pio_irq_id);
}

void cvideo_init_irq(PIO pio, uint data_pin, uint sync_pin, cvideo_data_callback_t callback) {
    if (CVIDEO_PIX_PER_LINE % 32 != 0) {
        printf("ERROR: Horizontal pixel count must be a multiple of 32\r\n");
    }
    else {
        cvideo_pio = pio;
        data_callback = callback;
        if (pio_get_index(pio) == 1){
            pio_irq_id = PIO1_IRQ_1;
        }

        // Run the data clock 32x faster than needed to reduce horizontal jitter due to synchronisation between SMs
        float data_clockdiv = (clock_get_hz(clk_sys) / (CVIDEO_PIX_PER_LINE / DATA_INTERVAL)) / CLOCKS_PER_BIT;
        float sync_clockdiv = clock_get_hz(clk_sys) * SYNC_INTERVAL;
        
        if (data_clockdiv < 1) {
            printf("WARNING: PIO data SM clock divider (value %f) less than 1\r\n", data_clockdiv);
        }

        printf("Data clockdiv %f\r\n", data_clockdiv);
        printf("Sync clockdiv %f\r\n", sync_clockdiv);
        
        uint offset_sync = pio_add_program(pio, &cvsync_program);
        uint offset_data = pio_add_program(pio, &cvdata_program);

        cvdata_program_init(pio, DATA_SM_ID, offset_data, data_clockdiv, data_pin);
        cvsync_program_init(pio, SYNC_SM_ID, offset_sync, sync_clockdiv, sync_pin);

        pio_sm_set_enabled(pio, DATA_SM_ID, true);
        pio_sm_set_enabled(pio, SYNC_SM_ID, true);

        // Enable FIFO refill interrupt for data state machine
        irq_set_enabled(pio_irq_id, true);
        irq_set_exclusive_handler(pio_irq_id, cvdata_isr);
        irq_set_priority(pio_irq_id, 0);
        cvideo_pio->inte1 = 1 << 4 + DATA_SM_ID;
    }
}

static uint32_t * address_pointer; // = &viddata[0][0];

void cvideo_init_dma(PIO pio, uint data_pin, uint sync_pin, void *frame) {
    if (CVIDEO_PIX_PER_LINE % 32 != 0) {
        printf("ERROR: Horizontal pixel count must be a multiple of 32\r\n");
    }
    else {
        cvideo_pio = pio;

        // Run the data clock 32x faster than needed to reduce horizontal jitter due to synchronisation between SMs
        float data_clockdiv = (clock_get_hz(clk_sys) / (CVIDEO_PIX_PER_LINE / DATA_INTERVAL)) / CLOCKS_PER_BIT;
        float sync_clockdiv = clock_get_hz(clk_sys) * SYNC_INTERVAL;

        if (data_clockdiv < 1) {
            printf("WARNING: PIO data SM clock divider (value %f) less than 1\r\n", data_clockdiv);
        }

        printf("Data clockdiv %f\r\n", data_clockdiv);
        printf("Sync clockdiv %f\r\n", sync_clockdiv);

        uint offset_sync = pio_add_program(pio, &cvsync_program);
        uint offset_data = pio_add_program(pio, &cvdata_program);

        cvdata_program_init(pio, DATA_SM_ID, offset_data, data_clockdiv, data_pin);
        cvsync_program_init(pio, SYNC_SM_ID, offset_sync, sync_clockdiv, sync_pin);

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // ===========================-== DMA Data Channels =================================================
        /////////////////////////////////////////////////////////////////////////////////////////////////////

        // DMA channels - 0 sends screen data, 1 reconfigures and restarts 0
        int chan_0 = 0;
        int chan_1 = 1;
        int sm = DATA_SM_ID;

        address_pointer = frame;

        // Channel Zero (sends color data to PIO VGA machine)
        dma_channel_config c0 = dma_channel_get_default_config(chan_0);  // default configs
        channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);              // 8-bit txfers
        channel_config_set_read_increment(&c0, true);                        // yes read incrementing
        channel_config_set_write_increment(&c0, false);                      // no write incrementing;
        channel_config_set_dreq(&c0, pio_get_dreq(pio, sm, true)) ;                        // DREQ_PIO0_TX2 pacing (FIFO)
        channel_config_set_chain_to(&c0, chan_1);                        // chain to other channel

        dma_channel_configure(
            chan_0,                 // Channel to be configured
            &c0,                        // The configuration we just created
            &pio->txf[sm],          // write address (RGB PIO TX FIFO)
            frame,            // The initial read address (pixel color array)
            CVIDEO_LINES*CVIDEO_PIX_PER_LINE/32,                    // Number of transfers; in this case each is 1 byte.
            false                       // Don't start immediately.
        );

        // Channel One (reconfigures the first channel)
        dma_channel_config c1 = dma_channel_get_default_config(chan_1);   // default configs
        channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);              // 32-bit txfers
        channel_config_set_read_increment(&c1, false);                        // no read incrementing
        channel_config_set_write_increment(&c1, false);                       // no write incrementing
        channel_config_set_chain_to(&c1, chan_0);                         // chain to other channel

        dma_channel_configure(
            chan_1,                         // Channel to be configured
            &c1,                                // The configuration we just created
            &dma_hw->ch[chan_0].read_addr,  // Write address (channel 0 read address)
            &address_pointer,                   // Read address (POINTER TO AN ADDRESS)
            1,                                  // Number of transfers, in this case each is 4 byte
            false                               // Don't start immediately.
        );

        dma_start_channel_mask((1u << chan_0)) ;
    }
}

#endif




static inline void cvdata_program_init(PIO pio, uint sm, uint offset, float clockdiv, uint data_pin) {
    pio_sm_config c = cvdata_program_get_default_config(offset);

    // Map the state machine's OUT and SIDE pin group to one pin, namely the `pin`
    // parameter to this function.
    sm_config_set_set_pins(&c, data_pin, 1);
    sm_config_set_out_pins(&c, data_pin, 1);

    sm_config_set_clkdiv(&c, clockdiv);

    // Set this pin's GPIO function (connect PIO to the pad)
    pio_gpio_init(pio, data_pin);

    // Set the pin direction to output at the PIO
    pio_sm_set_consecutive_pindirs(pio, sm, data_pin, 1, true);

    // Enable autopull and set threshold to 32 bits
    // Note that register positions are the same for all state machines
    c.shiftctrl |= (1u<<PIO_SM0_SHIFTCTRL_AUTOPULL_LSB) |
                  (0u<<PIO_SM0_SHIFTCTRL_PULL_THRESH_LSB);

    
    // Load our configuration, and jump to the start of the program
    pio_sm_init(pio, sm, offset, &c);

    // Tell the state machine the number of pixels per line (minus 1)
    pio_sm_put(pio, sm, CVIDEO_PIX_PER_LINE - 1);
    pio_sm_exec(pio, sm, 0x80a0);  // pull
    pio_sm_exec(pio, sm, 0xa027);  // mov    x, osr 
    pio_sm_exec(pio, sm, 0x6060);  // out null 32 ; Discard OSR contents after copying to x

    // Set the state machine running
    pio_sm_set_enabled(pio, sm, true);
}

static inline void cvsync_program_init(PIO pio, uint sm, uint offset, float clockdiv, uint sync_pin) {
    pio_sm_config c = cvsync_program_get_default_config(offset);

    // Map the state machine's OUT and SIDE pin group to one pin, namely the `pin`
    // parameter to this function.
    sm_config_set_sideset_pins(&c, sync_pin);

    // Set the clock speed
    sm_config_set_clkdiv(&c, clockdiv);

    // Set this pin's GPIO function (connect PIO to the pad)
    pio_gpio_init(pio, sync_pin);

    // Set the pin direction to output at the PIO
    pio_sm_set_consecutive_pindirs(pio, sm, sync_pin, 1, true);

    // Load our configuration, and jump to the start of the program
    pio_sm_init(pio, sm, offset, &c);

    // Tell the state machine the number of video lines per frame (minus 1)
    pio_sm_put(pio, sm, (CVIDEO_LINES / 2) - 1);
    pio_sm_exec(pio, sm, 0x80a0);  // pull side 0

    // Set the state machine running
    pio_sm_set_enabled(pio, sm, true);
}
