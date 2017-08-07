
# add board to target lists
BL_BOARDS += discoveryf4
FW_BOARDS += discoveryf4
EF_BOARDS += discoveryf4
FT_BOARDS += discoveryf4
SIM_BOARDS += discoveryf4

# add dependencies for top level makefile
ef_discoveryf4_all: fw_discoveryf4_all bl_discoveryf4_all
fw_discoveryf4_all: bl_discoveryf4_all
ft_discoveryf4_all: ef_discoveryf4_all

fw_discoveryf4_program: fw_discoveryf4_all
bl_discoveryf4_program: bl_discoveryf4_all
ef_discoveryf4_program: ef_discoveryf4_all
