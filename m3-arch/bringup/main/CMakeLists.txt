# embedded/m3-arch/bringup/main/CMakeLists.txt

idf_component_register(SRCS "main.c"
                            "../../isr-ringbuf/src/ringbuf.c"
                            "../../frame_fsm/src/frame_fsm.c"
                        INCLUDE_DIRS "." "../../isr-ringbuf/src"
                            "../../frame_fsm/src"
                        REQUIRES esp_driver_gptimer
                        LDFRAGMENTS "linker.lf")
