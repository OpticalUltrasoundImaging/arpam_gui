### Build ImGui and find the backends (GLFW, Vulkan)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
list(APPEND VENDOR_LIBS "glfw")

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
# Add to vendor libs
list(APPEND VENDOR_LIBS "Vulkan::Vulkan")

# Dear ImGui
set(IMGUI_DIR ./imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
set(IMGUI_SRC 
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp)

add_library(imgui STATIC)
target_sources(imgui PRIVATE ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR} /backends)
target_link_libraries(imgui PUBLIC "glfw" "Vulkan::Vulkan")

list(APPEND VENDOR_LIBS "imgui")