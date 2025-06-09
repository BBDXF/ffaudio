project(
    'ffaudio',
    'c',
    version: '0.1',
    default_options: [
        'warning_level=3',
        # 'b_lto=true',
        # 'optimization=3',
        'c_std=c11',
    ],
)

cc = meson.get_compiler('c')

# 先尝试系统查找，找不到就指定一个固定的
ffmpeg_libs = ['libavcodec', 'libavformat', 'libavdevice', 'libavfilter', 'libavutil', 'libswscale', 'libswresample']
ffmpeg_dep = dependency(ffmpeg_libs, allow_fallback: false)
if not ffmpeg_dep.found()
    message('ffmpeg_dep not found in system!')
endif

openal_dep = dependency(['openal'], allow_fallback: false)
if not openal_dep.found()
    message('openal_dep not found in system!')
endif

# 可执行文件目标
executable(
    'aldemo',
    ['src/aldemo.c'],
    dependencies: [openal_dep],
    install: true,
    install_dir: meson.project_source_root() / 'out',
)

# lib 源文件
lib_sources = ['src/ffaudio.c']

# 动态库
lib = both_libraries(
    'ffaudio',
    lib_sources,
    version: '1.0',
    include_directories: include_directories('src'),
    dependencies: [ffmpeg_dep],
    install: true,
    install_dir: meson.project_source_root() / 'out',
)

shared_lib = lib.get_shared_lib()

install_headers(
    'src/ffaudio.h',
    install_dir: meson.project_source_root() / 'out',
)

# demo 源文件
demo_sources = ['demo.c']

# 可执行文件目标
demo = executable(
    'demo',
    demo_sources,
    link_with: lib.get_static_lib(),
    include_directories: include_directories('src'),
    install: true,
    install_dir: meson.project_source_root() / 'out',
)

# 测试目标
test(
    'ffaudio_test',
    demo,
    args: ['./res/test1.mp3'],
    workdir: meson.project_source_root(),
)
