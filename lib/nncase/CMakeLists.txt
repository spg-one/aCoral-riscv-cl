include_directories(${SDK_ROOT}/third_party/xtl/include ${CMAKE_CURRENT_LIST_DIR}/nncase/include) #//SPG这个nncase的include路径可能有问题

FILE(GLOB_RECURSE NNCASE_SRC
        "${CMAKE_CURRENT_LIST_DIR}/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
        )

ADD_LIBRARY(nncase
        ${NNCASE_SRC}
        )
TARGET_COMPILE_OPTIONS(nncase PRIVATE -O2)