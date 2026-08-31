cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hyperram)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_TEST_PSRAM_FLASH_COPY app PRIVATE src/test_flash_to_psram.c)
target_sources_ifdef(CONFIG_TEST_PSRAM app PRIVATE src/test_hyperram.c)
target_sources_ifdef(CONFIG_TEST_HYPERRAM app PRIVATE src/test_s80ks2564.c)
