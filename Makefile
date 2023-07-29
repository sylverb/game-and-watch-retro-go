TARGET = gw_retro_go

DEBUG = 0

OPT = -O2 -ggdb3

# To enable verbose, append VERBOSE=1 to make, e.g.:
# make VERBOSE=1
ifneq ($(strip $(VERBOSE)),1)
V = @
endif

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Src/bilinear.c \
Core/Src/gw_buttons.c \
Core/Src/gw_flash.c \
Core/Src/gw_lcd.c \
Core/Src/gw_malloc.c \
Core/Src/game_genie.c \
Core/Src/main.c \
Core/Src/sha256.c \
Core/Src/flashapp.c \
Core/Src/bq24072.c \
Core/Src/filesystem.c \
Core/Src/porting/lib/lzma/LzmaDec.c \
Core/Src/porting/lib/lzma/lzma.c \
Core/Src/porting/lib/hw_jpeg_decoder.c \
Core/Src/porting/lib/littlefs/lfs.c \
Core/Src/porting/lib/littlefs/lfs_util.c \
Core/Src/porting/common.c \
Core/Src/porting/odroid_audio.c \
Core/Src/porting/odroid_display.c \
Core/Src/porting/odroid_input.c \
Core/Src/porting/odroid_netplay.c \
Core/Src/porting/odroid_overlay.c \
Core/Src/porting/odroid_sdcard.c \
Core/Src/porting/odroid_system.c \
Core/Src/porting/crc32.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/system_stm32h7xx.c

TAMP_DIR = Core/Src/porting/lib/tamp/tamp/_c_src/
TAMP_C_SOURCES = \
$(TAMP_DIR)/tamp/common.c \
$(TAMP_DIR)/tamp/compressor.c \
$(TAMP_DIR)/tamp/decompressor.c

GNUBOY_C_SOURCES = \
Core/Src/porting/gb/main_gb.c \
retro-go-stm32/gnuboy-go/components/gnuboy/cpu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/debug.c \
retro-go-stm32/gnuboy-go/components/gnuboy/emu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/hw.c \
retro-go-stm32/gnuboy-go/components/gnuboy/lcd.c \
retro-go-stm32/gnuboy-go/components/gnuboy/loader.c \
retro-go-stm32/gnuboy-go/components/gnuboy/mem.c \
retro-go-stm32/gnuboy-go/components/gnuboy/rtc.c \
retro-go-stm32/gnuboy-go/components/gnuboy/sound.c \

NES_C_SOURCES = \
Core/Src/porting/nes/main_nes.c \
Core/Src/porting/nes/nofrendo_stm32.c \
retro-go-stm32/nofrendo-go/components/nofrendo/cpu/dis6502.c \
retro-go-stm32/nofrendo-go/components/nofrendo/cpu/nes6502.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map000.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map001.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map002.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map003.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map004.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map005.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map007.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map008.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map009.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map010.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map011.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map015.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map016.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map018.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map019.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map021.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map020.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map022.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map023.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map024.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map030.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map032.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map033.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map034.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map040.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map041.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map042.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map046.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map050.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map064.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map065.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map066.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map070.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map071.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map073.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map074.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map075.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map076.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map078.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map079.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map085.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map087.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map093.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map094.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map119.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map160.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map162.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map185.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map191.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map192.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map193.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map194.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map195.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map228.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map206.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map229.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map231.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map252.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map253.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_apu.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_input.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mem.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mmc.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_ppu.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_rom.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_state.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes.c

NES_FCEU_C_SOURCES = \
Core/Src/porting/nes_fceu/main_nes_fceu.c \
fceumm-go/src/boards/09-034a.c \
fceumm-go/src/boards/3d-block.c \
fceumm-go/src/boards/8in1.c \
fceumm-go/src/boards/12in1.c \
fceumm-go/src/boards/15.c \
fceumm-go/src/boards/18.c \
fceumm-go/src/boards/28.c \
fceumm-go/src/boards/31.c \
fceumm-go/src/boards/32.c \
fceumm-go/src/boards/33.c \
fceumm-go/src/boards/34.c \
fceumm-go/src/boards/40.c \
fceumm-go/src/boards/41.c \
fceumm-go/src/boards/42.c \
fceumm-go/src/boards/43.c \
fceumm-go/src/boards/46.c \
fceumm-go/src/boards/50.c \
fceumm-go/src/boards/51.c \
fceumm-go/src/boards/57.c \
fceumm-go/src/boards/60.c \
fceumm-go/src/boards/62.c \
fceumm-go/src/boards/65.c \
fceumm-go/src/boards/67.c \
fceumm-go/src/boards/68.c \
fceumm-go/src/boards/69.c \
fceumm-go/src/boards/71.c \
fceumm-go/src/boards/72.c \
fceumm-go/src/boards/77.c \
fceumm-go/src/boards/79.c \
fceumm-go/src/boards/80.c \
fceumm-go/src/boards/82.c \
fceumm-go/src/boards/88.c \
fceumm-go/src/boards/91.c \
fceumm-go/src/boards/96.c \
fceumm-go/src/boards/99.c \
fceumm-go/src/boards/103.c \
fceumm-go/src/boards/104.c \
fceumm-go/src/boards/106.c \
fceumm-go/src/boards/108.c \
fceumm-go/src/boards/112.c \
fceumm-go/src/boards/116.c \
fceumm-go/src/boards/117.c \
fceumm-go/src/boards/120.c \
fceumm-go/src/boards/121.c \
fceumm-go/src/boards/126-422-534.c \
fceumm-go/src/boards/134.c \
fceumm-go/src/boards/151.c \
fceumm-go/src/boards/156.c \
fceumm-go/src/boards/162.c \
fceumm-go/src/boards/163.c \
fceumm-go/src/boards/164.c \
fceumm-go/src/boards/168.c \
fceumm-go/src/boards/170.c \
fceumm-go/src/boards/175.c \
fceumm-go/src/boards/177.c \
fceumm-go/src/boards/178.c \
fceumm-go/src/boards/183.c \
fceumm-go/src/boards/185.c \
fceumm-go/src/boards/186.c \
fceumm-go/src/boards/187.c \
fceumm-go/src/boards/189.c \
fceumm-go/src/boards/190.c \
fceumm-go/src/boards/193.c \
fceumm-go/src/boards/195.c \
fceumm-go/src/boards/199.c \
fceumm-go/src/boards/206.c \
fceumm-go/src/boards/208.c \
fceumm-go/src/boards/218.c \
fceumm-go/src/boards/222.c \
fceumm-go/src/boards/225.c \
fceumm-go/src/boards/228.c \
fceumm-go/src/boards/230.c \
fceumm-go/src/boards/232.c \
fceumm-go/src/boards/233.c \
fceumm-go/src/boards/234.c \
fceumm-go/src/boards/235.c \
fceumm-go/src/boards/236.c \
fceumm-go/src/boards/237.c \
fceumm-go/src/boards/244.c \
fceumm-go/src/boards/246.c \
fceumm-go/src/boards/252.c \
fceumm-go/src/boards/253.c \
fceumm-go/src/boards/267.c \
fceumm-go/src/boards/268.c \
fceumm-go/src/boards/269.c \
fceumm-go/src/boards/272.c \
fceumm-go/src/boards/283.c \
fceumm-go/src/boards/291.c \
fceumm-go/src/boards/293.c \
fceumm-go/src/boards/294.c \
fceumm-go/src/boards/310.c \
fceumm-go/src/boards/319.c \
fceumm-go/src/boards/326.c \
fceumm-go/src/boards/330.c \
fceumm-go/src/boards/334.c \
fceumm-go/src/boards/351.c \
fceumm-go/src/boards/353.c \
fceumm-go/src/boards/354.c \
fceumm-go/src/boards/356.c \
fceumm-go/src/boards/357.c \
fceumm-go/src/boards/359.c \
fceumm-go/src/boards/360.c \
fceumm-go/src/boards/364.c \
fceumm-go/src/boards/368.c \
fceumm-go/src/boards/369.c \
fceumm-go/src/boards/370.c \
fceumm-go/src/boards/372.c \
fceumm-go/src/boards/375.c \
fceumm-go/src/boards/376.c \
fceumm-go/src/boards/377.c \
fceumm-go/src/boards/380.c \
fceumm-go/src/boards/382.c \
fceumm-go/src/boards/383.c \
fceumm-go/src/boards/389.c \
fceumm-go/src/boards/390.c \
fceumm-go/src/boards/391.c \
fceumm-go/src/boards/393.c \
fceumm-go/src/boards/395.c \
fceumm-go/src/boards/396.c \
fceumm-go/src/boards/401.c \
fceumm-go/src/boards/403.c \
fceumm-go/src/boards/410.c \
fceumm-go/src/boards/411.c \
fceumm-go/src/boards/414.c \
fceumm-go/src/boards/416.c \
fceumm-go/src/boards/417.c \
fceumm-go/src/boards/428.c \
fceumm-go/src/boards/431.c \
fceumm-go/src/boards/432.c \
fceumm-go/src/boards/433.c \
fceumm-go/src/boards/434.c \
fceumm-go/src/boards/436.c \
fceumm-go/src/boards/437.c \
fceumm-go/src/boards/438.c \
fceumm-go/src/boards/441.c \
fceumm-go/src/boards/443.c \
fceumm-go/src/boards/444.c \
fceumm-go/src/boards/449.c \
fceumm-go/src/boards/452.c \
fceumm-go/src/boards/455.c \
fceumm-go/src/boards/456.c \
fceumm-go/src/boards/460.c \
fceumm-go/src/boards/463.c \
fceumm-go/src/boards/465.c \
fceumm-go/src/boards/466.c \
fceumm-go/src/boards/467.c \
fceumm-go/src/boards/468.c \
fceumm-go/src/boards/516.c \
fceumm-go/src/boards/533.c \
fceumm-go/src/boards/539.c \
fceumm-go/src/boards/554.c \
fceumm-go/src/boards/558.c \
fceumm-go/src/boards/603-5052.c \
fceumm-go/src/boards/8157.c \
fceumm-go/src/boards/8237.c \
fceumm-go/src/boards/411120-c.c \
fceumm-go/src/boards/830118C.c \
fceumm-go/src/boards/830134C.c \
fceumm-go/src/boards/a9746.c \
fceumm-go/src/boards/ac-08.c \
fceumm-go/src/boards/addrlatch.c \
fceumm-go/src/boards/ax40g.c \
fceumm-go/src/boards/ax5705.c \
fceumm-go/src/boards/bandai.c \
fceumm-go/src/boards/bb.c \
fceumm-go/src/boards/bj56.c \
fceumm-go/src/boards/bmc42in1r.c \
fceumm-go/src/boards/bmc64in1nr.c \
fceumm-go/src/boards/bmc60311c.c \
fceumm-go/src/boards/bmc80013b.c \
fceumm-go/src/boards/bmc830425C4391t.c \
fceumm-go/src/boards/bmcctc09.c \
fceumm-go/src/boards/bmcgamecard.c \
fceumm-go/src/boards/bmck3006.c \
fceumm-go/src/boards/bmck3033.c \
fceumm-go/src/boards/bmck3036.c \
fceumm-go/src/boards/bmcl6in1.c \
fceumm-go/src/boards/BMW8544.c \
fceumm-go/src/boards/bonza.c \
fceumm-go/src/boards/bs-5.c \
fceumm-go/src/boards/cheapocabra.c \
fceumm-go/src/boards/cityfighter.c \
fceumm-go/src/boards/coolgirl.c \
fceumm-go/src/boards/dance2000.c \
fceumm-go/src/boards/datalatch.c \
fceumm-go/src/boards/dream.c \
fceumm-go/src/boards/edu2000.c \
fceumm-go/src/boards/eeprom_93C66.c \
fceumm-go/src/boards/eh8813a.c \
fceumm-go/src/boards/et-100.c \
fceumm-go/src/boards/et-4320.c \
fceumm-go/src/boards/f-15.c \
fceumm-go/src/boards/fceu-emu2413.c \
fceumm-go/src/boards/famicombox.c \
fceumm-go/src/boards/faridunrom.c \
fceumm-go/src/boards/ffe.c \
fceumm-go/src/boards/fk23c.c \
fceumm-go/src/boards/gn26.c \
fceumm-go/src/boards/h2288.c \
fceumm-go/src/boards/hp10xx_hp20xx.c \
fceumm-go/src/boards/hp898f.c \
fceumm-go/src/boards/jyasic.c \
fceumm-go/src/boards/karaoke.c \
fceumm-go/src/boards/KG256.c \
fceumm-go/src/boards/kof97.c \
fceumm-go/src/boards/KS7012.c \
fceumm-go/src/boards/KS7013.c \
fceumm-go/src/boards/KS7016.c \
fceumm-go/src/boards/KS7017.c \
fceumm-go/src/boards/KS7030.c \
fceumm-go/src/boards/KS7031.c \
fceumm-go/src/boards/KS7032.c \
fceumm-go/src/boards/KS7037.c \
fceumm-go/src/boards/KS7057.c \
fceumm-go/src/boards/le05.c \
fceumm-go/src/boards/lh32.c \
fceumm-go/src/boards/lh51.c \
fceumm-go/src/boards/lh53.c \
fceumm-go/src/boards/malee.c \
fceumm-go/src/boards/mihunche.c \
fceumm-go/src/boards/mmc1.c \
fceumm-go/src/boards/mmc2and4.c \
fceumm-go/src/boards/mmc3.c \
fceumm-go/src/boards/mmc5.c \
fceumm-go/src/boards/n106.c \
fceumm-go/src/boards/n625092.c \
fceumm-go/src/boards/novel.c \
fceumm-go/src/boards/onebus.c \
fceumm-go/src/boards/pec-586.c \
fceumm-go/src/boards/resetnromxin1.c \
fceumm-go/src/boards/resettxrom.c \
fceumm-go/src/boards/rt-01.c \
fceumm-go/src/boards/SA-9602B.c \
fceumm-go/src/boards/sachen.c \
fceumm-go/src/boards/sheroes.c \
fceumm-go/src/boards/sl1632.c \
fceumm-go/src/boards/subor.c \
fceumm-go/src/boards/super40in1.c \
fceumm-go/src/boards/supervision.c \
fceumm-go/src/boards/t-227-1.c \
fceumm-go/src/boards/t-262.c \
fceumm-go/src/boards/tengen.c \
fceumm-go/src/boards/tf-1201.c \
fceumm-go/src/boards/transformer.c \
fceumm-go/src/boards/txcchip.c \
fceumm-go/src/boards/unrom512.c \
fceumm-go/src/boards/vrc1.c \
fceumm-go/src/boards/vrc2and4.c \
fceumm-go/src/boards/vrc3.c \
fceumm-go/src/boards/vrc6.c \
fceumm-go/src/boards/vrc7.c \
fceumm-go/src/boards/vrc7p.c \
fceumm-go/src/boards/yoko.c \
fceumm-go/src/cheat.c \
fceumm-go/src/fceu-cart.c \
fceumm-go/src/fceu-endian.c \
fceumm-go/src/fceu-memory.c \
fceumm-go/src/fceu-sound.c \
fceumm-go/src/fceu-state.c \
fceumm-go/src/fceu.c \
fceumm-go/src/fds.c \
fceumm-go/src/fds_apu.c \
fceumm-go/src/filter.c \
fceumm-go/src/general.c \
fceumm-go/src/ines.c \
fceumm-go/src/input.c \
fceumm-go/src/md5.c \
fceumm-go/src/nsf.c \
fceumm-go/src/palette.c \
fceumm-go/src/ppu.c \
fceumm-go/src/video.c \
fceumm-go/src/x6502.c \

SMSPLUSGX_C_SOURCES = \
retro-go-stm32/smsplusgx-go/components/smsplus/loadrom.c \
retro-go-stm32/smsplusgx-go/components/smsplus/render.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sms.c \
retro-go-stm32/smsplusgx-go/components/smsplus/state.c \
retro-go-stm32/smsplusgx-go/components/smsplus/vdp.c \
retro-go-stm32/smsplusgx-go/components/smsplus/pio.c \
retro-go-stm32/smsplusgx-go/components/smsplus/tms.c \
retro-go-stm32/smsplusgx-go/components/smsplus/memz80.c \
retro-go-stm32/smsplusgx-go/components/smsplus/system.c \
retro-go-stm32/smsplusgx-go/components/smsplus/cpu/z80.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/emu2413.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/fmintf.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/sn76489.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/sms_sound.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/ym2413.c \
Core/Src/porting/smsplusgx/main_smsplusgx.c

PCE_C_SOURCES = \
retro-go-stm32/pce-go/components/pce-go/gfx.c \
retro-go-stm32/pce-go/components/pce-go/h6280.c \
retro-go-stm32/pce-go/components/pce-go/pce.c \
Core/Src/porting/pce/sound_pce.c \
Core/Src/porting/pce/main_pce.c

CORE_MSX = blueMSX-go
LIBRETRO_COMM_DIR  = $(CORE_MSX)/libretro-common

MSX_C_SOURCES = \
$(CORE_MSX)/Src/Libretro/Timer.c \
$(CORE_MSX)/Src/Libretro/Emulator.c \
$(CORE_MSX)/Src/Bios/Patch.c \
$(CORE_MSX)/Src/Memory/DeviceManager.c \
$(CORE_MSX)/Src/Memory/IoPort.c \
$(CORE_MSX)/Src/Memory/MegaromCartridge.c \
$(CORE_MSX)/Src/Memory/ramNormal.c \
$(CORE_MSX)/Src/Memory/ramMapper.c \
$(CORE_MSX)/Src/Memory/ramMapperIo.c \
$(CORE_MSX)/Src/Memory/RomLoader.c \
$(CORE_MSX)/Src/Memory/romMapperASCII8.c \
$(CORE_MSX)/Src/Memory/romMapperASCII16.c \
$(CORE_MSX)/Src/Memory/romMapperASCII16nf.c \
$(CORE_MSX)/Src/Memory/romMapperBasic.c \
$(CORE_MSX)/Src/Memory/romMapperCasette.c \
$(CORE_MSX)/Src/Memory/romMapperDRAM.c \
$(CORE_MSX)/Src/Memory/romMapperF4device.c \
$(CORE_MSX)/Src/Memory/romMapperKoei.c \
$(CORE_MSX)/Src/Memory/romMapperKonami4.c \
$(CORE_MSX)/Src/Memory/romMapperKonami4nf.c \
$(CORE_MSX)/Src/Memory/romMapperKonami5.c \
$(CORE_MSX)/Src/Memory/romMapperLodeRunner.c \
$(CORE_MSX)/Src/Memory/romMapperMsxDos2.c \
$(CORE_MSX)/Src/Memory/romMapperMsxMusic.c \
$(CORE_MSX)/Src/Memory/romMapperNormal.c \
$(CORE_MSX)/Src/Memory/romMapperPlain.c \
$(CORE_MSX)/Src/Memory/romMapperRType.c \
$(CORE_MSX)/Src/Memory/romMapperStandard.c \
$(CORE_MSX)/Src/Memory/romMapperSunriseIDE.c \
$(CORE_MSX)/Src/Memory/romMapperSCCplus.c \
$(CORE_MSX)/Src/Memory/romMapperTC8566AF.c \
$(CORE_MSX)/Src/Memory/SlotManager.c \
$(CORE_MSX)/Src/VideoChips/VDP_YJK.c \
$(CORE_MSX)/Src/VideoChips/VDP_MSX.c \
$(CORE_MSX)/Src/VideoChips/V9938.c \
$(CORE_MSX)/Src/VideoChips/VideoManager.c \
$(CORE_MSX)/Src/Z80/R800.c \
$(CORE_MSX)/Src/Z80/R800SaveState.c \
$(CORE_MSX)/Src/Input/JoystickPort.c \
$(CORE_MSX)/Src/Input/MsxJoystick.c \
$(CORE_MSX)/Src/IoDevice/Disk.c \
$(CORE_MSX)/Src/IoDevice/HarddiskIDE.c \
$(CORE_MSX)/Src/IoDevice/I8255.c \
$(CORE_MSX)/Src/IoDevice/MsxPPI.c \
$(CORE_MSX)/Src/IoDevice/RTC.c \
$(CORE_MSX)/Src/IoDevice/SunriseIDE.c \
$(CORE_MSX)/Src/IoDevice/TC8566AF.c \
$(CORE_MSX)/Src/SoundChips/AudioMixer.c \
$(CORE_MSX)/Src/SoundChips/AY8910.c \
$(CORE_MSX)/Src/SoundChips/SCC.c \
$(CORE_MSX)/Src/SoundChips/MsxPsg.c \
$(CORE_MSX)/Src/SoundChips/YM2413_msx.c \
$(CORE_MSX)/Src/SoundChips/emu2413_msx.c \
$(CORE_MSX)/Src/Emulator/AppConfig.c \
$(CORE_MSX)/Src/Emulator/LaunchFile.c \
$(CORE_MSX)/Src/Emulator/Properties.c \
$(CORE_MSX)/Src/Utils/IsFileExtension.c \
$(CORE_MSX)/Src/Utils/StrcmpNoCase.c \
$(CORE_MSX)/Src/Utils/TokenExtract.c \
$(CORE_MSX)/Src/Board/Board.c \
$(CORE_MSX)/Src/Board/Machine.c \
$(CORE_MSX)/Src/Board/MSX.c \
$(CORE_MSX)/Src/Input/InputEvent.c \
Core/Src/porting/msx/main_msx.c \
Core/Src/porting/msx/save_msx.c

GW_C_SOURCES = \
Core/Src/porting/lib/lz4_depack.c \
LCD-Game-Emulator/src/cpus/sm500op.c \
LCD-Game-Emulator/src/cpus/sm510op.c \
LCD-Game-Emulator/src/cpus/sm500core.c \
LCD-Game-Emulator/src/cpus/sm5acore.c \
LCD-Game-Emulator/src/cpus/sm510core.c \
LCD-Game-Emulator/src/cpus/sm511core.c \
LCD-Game-Emulator/src/cpus/sm510base.c \
LCD-Game-Emulator/src/gw_sys/gw_romloader.c \
LCD-Game-Emulator/src/gw_sys/gw_graphic.c \
LCD-Game-Emulator/src/gw_sys/gw_system.c \
Core/Src/porting/gw/main_gw.c

WSV_C_SOURCES = \
potator/common/controls.c \
potator/common/gpu.c \
potator/common/m6502/m6502.c \
potator/common/memorymap.c \
potator/common/timer.c \
potator/common/watara.c \
potator/common/wsv_sound.c \
Core/Src/porting/wsv/main_wsv.c

MD_C_SOURCES = \
gwenesis/src/cpus/M68K/m68kcpu.c \
gwenesis/src/cpus/Z80/Z80.c \
gwenesis/src/sound/z80inst.c \
gwenesis/src/sound/ym2612.c \
gwenesis/src/sound/gwenesis_sn76489.c \
gwenesis/src/bus/gwenesis_bus.c \
gwenesis/src/io/gwenesis_io.c \
gwenesis/src/vdp/gwenesis_vdp_mem.c \
gwenesis/src/vdp/gwenesis_vdp_gfx.c \
gwenesis/src/savestate/gwenesis_savestate.c \
Core/Src/porting/gwenesis/main_gwenesis.c

A7800_C_SOURCES = \
prosystem-go/core/Bios.c \
prosystem-go/core/Cartridge.c \
prosystem-go/core/Database.c \
prosystem-go/core/Hash.c \
prosystem-go/core/Maria.c \
prosystem-go/core/Memory.c \
prosystem-go/core/Palette.c \
prosystem-go/core/Pokey.c \
prosystem-go/core/ProSystem.c \
prosystem-go/core/Region.c \
prosystem-go/core/Riot.c \
prosystem-go/core/Sally.c \
prosystem-go/core/Tia.c \
Core/Src/porting/a7800/main_a7800.c

AMSTRAD_C_SOURCES = \
caprice32-go/cap32/cap32.c \
caprice32-go/cap32/crtc.c \
caprice32-go/cap32/fdc.c \
caprice32-go/cap32/kbdauto.c \
caprice32-go/cap32/psg.c \
caprice32-go/cap32/slots.c \
caprice32-go/cap32/cap32_z80.c \
Core/Src/porting/amstrad/main_amstrad.c \
Core/Src/porting/amstrad/amstrad_catalog.c \
Core/Src/porting/amstrad/amstrad_format.c \
Core/Src/porting/amstrad/amstrad_loader.c \
Core/Src/porting/amstrad/amstrad_video8bpp.c

TAMP_C_INCLUDES += -I$(TAMP_DIR)

GNUBOY_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-I./

NES_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/cpu \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/mappers \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/nes \
-Iretro-go-stm32/nofrendo-go/components/nofrendo \
-Iretro-go-stm32/components/odroid \
-I./

NES_FCEU_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Ifceumm-go/src/ \
-I./

SMSPLUSGX_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-Iretro-go-stm32/smsplusgx-go/components/smsplus \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/cpu \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/sound \
-I./

PCE_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-Iretro-go-stm32/pce-go/components/pce-go \
-Iretro-go-stm32/smsplusgx-go/components/smsplus \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/cpu \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/sound \
-I./

GW_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-ILCD-Game-Emulator/src \
-ILCD-Game-Emulator/src/cpus \
-ILCD-Game-Emulator/src/gw_sys \
-I./

MD_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Igwenesis/src/cpus/M68K \
-Igwenesis/src/cpus/Z80 \
-Igwenesis/src/sound \
-Igwenesis/src/bus \
-Igwenesis/src/vdp \
-Igwenesis/src/io \
-Igwenesis/src/savestate \
-I./


C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-ICore/Src/porting/lib/littlefs/ \
-ICore/Src/porting/lib/tamp/tamp/_c_src \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/cpu \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/mappers \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/nes \
-Iretro-go-stm32/nofrendo-go/components/nofrendo \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-Iretro-go-stm32/smsplusgx-go/components/smsplus \
-I./

MSX_C_INCLUDES += \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-I$(CORE_MSX) \
-I$(LIBRETRO_COMM_DIR)/include \
-I$(CORE_MSX)/Src/Arch \
-I$(CORE_MSX)/Src/Bios \
-I$(CORE_MSX)/Src/Board \
-I$(CORE_MSX)/Src/BuildInfo \
-I$(CORE_MSX)/Src/Common \
-I$(CORE_MSX)/Src/Debugger \
-I$(CORE_MSX)/Src/Emulator \
-I$(CORE_MSX)/Src/IoDevice \
-I$(CORE_MSX)/Src/Language \
-I$(CORE_MSX)/Src/Media \
-I$(CORE_MSX)/Src/Memory \
-I$(CORE_MSX)/Src/Resources \
-I$(CORE_MSX)/Src/SoundChips \
-I$(CORE_MSX)/Src/TinyXML \
-I$(CORE_MSX)/Src/Utils \
-I$(CORE_MSX)/Src/VideoChips \
-I$(CORE_MSX)/Src/VideoRender \
-I$(CORE_MSX)/Src/Z80 \
-I$(CORE_MSX)/Src/Input \
-I$(CORE_MSX)/Src/Libretro \
-I./

WSV_C_INCLUDES += \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Ipotator/common \
-I./

A7800_C_INCLUDES += \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iprosystem-go/core \
-I./

AMSTRAD_C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-Iretro-go-stm32/components/odroid \
-Icaprice32-go/cap32 \
-I./

include Makefile.common


$(BUILD_DIR)/$(TARGET)_extflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j ._itcram_hot -j ._ram_exec -j ._extflash -j .overlay_nes -j .overlay_nes_fceu -j .overlay_gb -j .overlay_sms -j .overlay_col -j .overlay_pce -j .overlay_msx -j .overlay_gw -j .overlay_wsv -j .overlay_md -j .overlay_a7800 -j .overlay_amstrad $< $(BUILD_DIR)/$(TARGET)_extflash.bin

$(BUILD_DIR)/$(TARGET)_intflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j .isr_vector -j .text -j .rodata -j .ARM.extab -j .preinit_array -j .init_array -j .fini_array -j .data $< $(BUILD_DIR)/$(TARGET)_intflash.bin

$(BUILD_DIR)/$(TARGET)_intflash2.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j .flash2 $< $(BUILD_DIR)/$(TARGET)_intflash2.bin
