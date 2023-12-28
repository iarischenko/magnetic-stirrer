# Instructions 

* git clone https://github.com/iarischenko/magnetic-stirrer.git
* cd software
* (only once) git clone https://github.com/libopencm3/libopencm3 && bear -- make -C libopencm3/ TARGETS='stm32/f1'
* bear --append -- make -C stm32f103-nixie

bear is needed to create compile_commands.json and enable clangd


