cmake_minimum_required(VERSION 2.8)

puyoai_add_cxx_flags("-W -Wall -Wno-deprecated-declarations")

include_directories(..)
include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDLTTF_INCLUDE_DIR})
include_directories(${SDLIMAGE_INCLUDE_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})

if(V4L2_LIBRARY)
  puyoai_add_cxx_flags("-DUSE_V4L2")
endif()

add_library(puyoai_capture capture.cc images.cc movie.cc source.cc viddev.cc)

function(capture_add_executable exe)
  add_executable(${exe} ${exe}.cc)
  target_link_libraries(${exe} puyoai_capture)
  target_link_libraries(${exe} puyoai_duel)
  target_link_libraries(${exe} puyoai_core)
  puyoai_target_link_libraries(${exe})
  target_link_libraries(${exe} ${SDL_LIBRARY})
  target_link_libraries(${exe} ${SDLTTF_LIBRARY})
  target_link_libraries(${exe} ${SDLIMAGE_LIBRARY})
  target_link_libraries(${exe} ${FFMPEG_LIBRARIES})
  if(V4L2_LIBRARY)
    target_link_libraries(${exe} ${V4L2_LIBRARY})
  endif()
endfunction()

function(capture_add_test exe)
  capture_add_executable(${exe})
  target_link_libraries(${exe} gtest gtest_main)
  add_test(check-${exe} ${exe})
endfunction()

capture_add_executable(connect)
capture_add_executable(parse_bmp)
capture_add_executable(parse_imgs)
capture_add_executable(parse_movie)
if(V4L2_LIBRARY)
  capture_add_executable(parse_viddev)
endif()

include_directories(${gtest_SOURCE_DIR}/include
                    ${gtest_SOURCE_DIR})
capture_add_test(capture_test)

