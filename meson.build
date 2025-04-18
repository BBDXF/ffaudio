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
ffmpeg_libs = ['avcodec', 'avformat', 'avdevice', 'avfilter', 'avutil', 'swscale', 'swresample']
ffmpeg_dep = dependency(ffmpeg_libs, allow_fallback: false)
if ffmpeg_dep.found()
    message('ffmpeg_dep found library in system!')
else
    ffmpeg_lib_dir = 'C:/ffmpeg' #'C:/MyTools/scoop_root/apps/ffmpeg-shared/current'
    message('ffmpeg_dep try t found library in dir:', ffmpeg_lib_dir)
    ffmepg_lib_dir_lib = ffmpeg_lib_dir / 'lib'
    ffmepg_lib_dir_include = ffmpeg_lib_dir / 'include'

    ffmpeg_dep_libs = []
    foreach lib : ffmpeg_libs 
        ffmpeg_dep_libs += cc.find_library('lib', dirs: ffmepg_lib_dir_lib)
    endforeach

    ffmpeg_dep = declare_dependency(
        include_directories: include_directories(ffmepg_lib_dir_include),
        dependencies: ffmpeg_dep_libs,
    )
endif

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
