project(glass_flaw_detect_core)

file(GLOB ROOT_CPP_SRCS ./*.cpp)
file(GLOB_RECURSE SRCS 
  ./analyzer/*.cpp 
  ./io_card/*.cpp 
  ./rpc/*.cpp 
  ./rpc/*.cpp 
  ./rabbitmq/*.cpp 
  ./util/*.cpp 
  ./util/*.c 
  ./account/*.cpp 
  ./plc/*.cpp 
  ./db/*.cpp
  ./db/*.c)
file(GLOB_RECURSE CAMERA_SRCS ./camera/*.cpp)
file(GLOB_RECURSE MOCK_SRCS ./mock/*.cpp)

list(APPEND SRCS ${ROOT_CPP_SRCS})
if(BUILD_FOR_MOCK)
  list(APPEND SRCS ${MOCK_SRCS})
else()
  list(APPEND SRCS ${CAMERA_SRCS})
endif()

add_library(${PROJECT_NAME} SHARED ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} 
  ${PLC_LIBRARIES}
  ${LOG_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES}
  ${OPENCV_LIBRARIES}
  ${CUDNN_LIBRARIES}
  ${CUDA_LIBRARIES}
  ${HARDWARE_LIBRARIES}
  ${Additional_LIBRARIES}
  ${TRT_LIBRARIES})

file(COPY ./config.toml DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ./analyzer/detectors/edge_detect_labels.txt DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ./analyzer/detectors/edge_detect_model.onnx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ./analyzer/detectors/face_detect_labels.txt DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ./analyzer/detectors/face_detect_model.onnx DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
