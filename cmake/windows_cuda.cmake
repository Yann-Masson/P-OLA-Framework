# Windows + CUDA configuration helpers.

function(configure_cuda_windows)
    if(WIN32)
        # Set CUDA toolkit path before project declaration (adjust if your CUDA is installed elsewhere)
        set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.1")
        set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc.exe")
        set(CUDAToolkit_ROOT "${CUDA_TOOLKIT_ROOT_DIR}")
        set(CUDACXX "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc.exe")

        # Add CUDA to CMAKE_PREFIX_PATH
        list(APPEND CMAKE_PREFIX_PATH "${CUDA_TOOLKIT_ROOT_DIR}")
    endif()

    # Configure PyTorch to use TORCH_CUDA_ARCH_LIST instead of CMAKE_CUDA_ARCHITECTURES
    # 8.6 corresponds to Ampere generation
    set(TORCH_CUDA_ARCH_LIST "8.6")

    # Explicitly unset CMAKE_CUDA_ARCHITECTURES to avoid PyTorch warnings
    unset(CMAKE_CUDA_ARCHITECTURES)
endfunction()

function(configure_windows_torch_runtime)
    if(MSVC)
        file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
        foreach(tgt IN LISTS ARGN)
            add_custom_command(TARGET ${tgt} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS} $<TARGET_FILE_DIR:${tgt}>)
        endforeach()
    endif()
endfunction()
