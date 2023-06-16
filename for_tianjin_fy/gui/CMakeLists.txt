project(glass_flaw_detect_gui)

file(GLOB_RECURSE CPP_SRCS ./*.cpp ./*.c)
file(GLOB_RECURSE CPP_HEADERS ./*.h)

QT5_WRAP_CPP(MOC_SRCS ${CPP_HEADERS})
QT5_ADD_RESOURCES(QRC_SRCS
  ./resources/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${QRC_SRCS} ${CPP_SRCS} ${MOC_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} 
  ${LOG_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES}
  ${OPENCV_LIBRARIES}
  ${CUDNN_LIBRARIES}
  ${CUDA_LIBRARIES}
  ${HARDWARE_LIBRARIES}
  ${Additional_LIBRARIES}
  ${TRT_LIBRARIES}
  "glass_flaw_detect_core")
