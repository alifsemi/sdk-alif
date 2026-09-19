cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(video)

target_sources(app PRIVATE
	src/main.c
	src/video_api.c
	src/video_common.c
)

# OV5640-only suite: omitted entirely on MT9M114 / ARX3A0 builds so those
# boards never see skipped ov5640_* tests in the ztest summary.
if(CONFIG_DT_HAS_OVTI_OV5640_ENABLED)
	target_sources(app PRIVATE src/video_test_ov5640.c)
endif()
