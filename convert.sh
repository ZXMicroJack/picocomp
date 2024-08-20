#!/bin/bash

dontdo2()
{
cat << EOF
           KR : in  STD_LOGIC_VECTOR (7 downto 0);
           KS : out  STD_LOGIC_VECTOR (7 downto 0);
	signal KR_bin : std_logic_vector(2 downto 0);
	
	signal row, col : std_logic_vector(2 downto 0);
	signal scan_code, scan_code_int : std_logic_vector(7 downto 0);

			KEY_CODE <= special & scan_code(6 downto 0);
EOF
}

filterreg()
{
#tr -d , | sed 's/conv_std_logic_vector(//g' | awk '
#{
#	printf "#define GKEY_%s 0x%s%s\n", toupper($13), $6, $10
#}
#'

#tr -d \", | sed 's/conv_std_logic_vector(//g' | awk '
#{
#	printf "#define KEY_%s 0%s\n", toupper($13), tolower($2)
#}
#'

#tr -d , | sed 's/conv_std_logic_vector(//g' | awk '
#{
#	printf "#define GKEY_%s %c%s%c\n", toupper($13), 39, toupper($13), 39
#}
#'

tr -d , | sed 's/conv_std_logic_vector(//g' | awk '
{
	printf "KEYMAP(%s,%s)\n", toupper($13), tolower($13)
}
'

}

filterreg2()
{
awk '
{
	print $3
}
'
}

cat << EOF | filterreg
					when X"1C" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(0, 3);	-- A
					when X"32" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(0, 3);	-- B
					when X"21" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(0, 3);	-- C
					when X"23" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(0, 3);	-- D
					when X"24" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(0, 3);	-- E
					when X"2B" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(0, 3);	-- F
					when X"34" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(0, 3);	-- G					
					when X"33" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(1, 3);	-- H
					when X"43" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(1, 3);	-- I
					when X"3B" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(1, 3);	-- J
					when X"42" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(1, 3);	-- K
					when X"4B" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(1, 3);	-- L
					when X"3A" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(1, 3);	-- M
					when X"31" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(1, 3);	-- N
					when X"44" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(1, 3);	-- O				
					when X"4D" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(2, 3);	-- P
					when X"15" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(2, 3);	-- Q
					when X"2D" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(2, 3);	-- R
					when X"1B" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(2, 3);	-- S
					when X"2C" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(2, 3);	-- T
					when X"3C" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(2, 3);	-- U
					when X"2A" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(2, 3);	-- V
					when X"1D" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(2, 3);	-- W				
					when X"22" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(3, 3);	-- X
					when X"35" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(3, 3);	-- Y
					when X"1A" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(3, 3);	-- Z					
					when X"29" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(3, 3);	-- SPACE				
					when X"45" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(4, 3);	-- 0
					when X"16" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(4, 3);	-- 1
					when X"1E" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(4, 3);	-- 2
					when X"26" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(4, 3);	-- 3
					when X"25" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(4, 3);	-- 4
					when X"2E" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(4, 3);	-- 5
					when X"36" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(4, 3);	-- 6
					when X"3D" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(4, 3);	-- 7				
					when X"3E" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(5, 3);	-- 8
					when X"46" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(5, 3);	-- 9
					when X"4C" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(5, 3);	-- SEMI
					when X"54" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(5, 3);	-- COLON (PS2 equ = [)
					when X"41" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(5, 3);	-- COMMA
					when X"55" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(5, 3);	-- EQU					
					when X"71" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(5, 3);	-- DOT
					when X"49" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(5, 3);	-- COMMA						
					when X"5A" => row <= conv_std_logic_vector(0, 3); col <= conv_std_logic_vector(6, 3);	-- retURN					
					when X"12" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(6, 3);	-- Lshift (left)
					when X"59" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(6, 3);	-- Rshift (right)								
					when X"4A" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(5, 3);	-- SLASH1							
EOF

cat << EOF | filterreg
					when X"75" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(3, 3);	-- UP
					when X"72" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(3, 3);	-- DOWN
					when X"6B" => row <= conv_std_logic_vector(5, 3); col <= conv_std_logic_vector(3, 3);	-- LEFT
					when X"74" => row <= conv_std_logic_vector(6, 3); col <= conv_std_logic_vector(3, 3);	-- RIGHT
					when X"4A" => row <= conv_std_logic_vector(7, 3); col <= conv_std_logic_vector(5, 3);	-- SLASH2			
					when X"69" => row <= conv_std_logic_vector(1, 3); col <= conv_std_logic_vector(6, 3);	-- BREAK end
					when X"6C" => row <= conv_std_logic_vector(2, 3); col <= conv_std_logic_vector(6, 3);	-- REPEAT home
					when X"71" => row <= conv_std_logic_vector(3, 3); col <= conv_std_logic_vector(6, 3);	-- DELETE
					when X"7D" => row <= conv_std_logic_vector(4, 3); col <= conv_std_logic_vector(6, 3);	-- LIST pageup
EOF

dontdo()
{
cat << EOF
LINE IN

			if (LINE_IN = '1') then
				if (KR_bin /= "000") then
					KS(0) <= key_array(conv_integer("000" & KR_bin));
				else
					KS(0) <= '1';
				end if;
			else
				KS(0) <= '0';
			end if;


			KS(1) <= key_array(conv_integer("001" & KR_bin));
			KS(2) <= key_array(conv_integer("010" & KR_bin));
			KS(3) <= key_array(conv_integer("011" & KR_bin));
			KS(4) <= key_array(conv_integer("100" & KR_bin));
			KS(5) <= key_array(conv_integer("101" & KR_bin));
			KS(6) <= key_array(conv_integer("110" & KR_bin));
			KS(7) <= key_array(conv_integer("111" & KR_bin));
EOF
}



