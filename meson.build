project(
  'vin',
  'cpp',
  default_options: [
    'c_std=c23',
    'cpp_std=c++23',
    'warning_level=3',
    'werror=true',
  ],
  version: '1.0rc1',
  license: 'MIT',
  meson_version : '>= 1.3',
  license_files: 'LICENSE',
)

vin_bin_linker_args = []
if build_machine.subsystem() == 'macos'
  bin_install_dir = 'bin'
  shared_lib_install_dir = 'lib'
  vin_lib_dir = '/opt/homebrew/Cellar/vin/' + meson.project_version() + '/lib/vin_libs'
  vin_bin_linker_args += ['-sectcreate', '__TEXT', '__info_plist', files('src/Info.plist')[0].full_path()]
else
  bin_install_dir = '/usr/bin'
  shared_lib_install_dir = '/usr/lib'
  vin_lib_dir = shared_lib_install_dir + '/lib/vin_libs'
endif

add_global_arguments('-DVIN_VERSION='+meson.project_version(), language : 'cpp')
add_global_arguments('-DVIN_LIB_DIR='+vin_lib_dir, language : 'cpp')
##########################################
####### Find the needed packages #########
##########################################
pkg_fn = import('pkgconfig')
fs_fn = import('fs')
qt6_fn = import('qt6')

arrow_dep = dependency('arrow')
catch_dep = dependency('catch2-with-main', required : false)
fdag_dep = dependency('functional_dag', method : 'pkg-config', version : '>=1.0')
flatbuffers_dep = dependency('flatbuffers')
glog_dep = dependency('glog')
qt6core_dep = dependency('qt6', modules : 'Core')
qt6gui_dep = dependency('qt6', modules : 'Gui')
qt6widgets_dep = dependency('qt6', modules : 'Widgets')
qt6multim_dep = dependency('qt6', modules : 'Multimedia')

if fs_fn.is_dir('build/include') == false
  run_command(find_program('mkdir'), '-p', 'build/include', check: true)
endif

########################################
####### Building specification #########
########################################

error_codes_h = custom_target(
    'error_codes_generate',
    output : 'error_codes.h',
    input : 'flatbufs/error_codes.fbs',
    command : ['flatc', '--reflect-types', '-o', 'include/vin/', '--no-prefix', '--filename-suffix', '', '--cpp', '@INPUT@'],
)

# Add the generated header to the include paths
vin_bin_srcs_cli_only = ['src/vin/main.cpp', error_codes_h]
vin_bin_deps_cli_only = [fdag_dep, qt6core_dep, glog_dep]

vin_bin_srcs_full = vin_bin_srcs_cli_only + ['src/vin/vin_gui.cpp', 'src/vin/vin_dag_manager.cpp']
vin_bin_deps_full = vin_bin_deps_cli_only + [qt6widgets_dep, qt6gui_dep]

if get_option('CLI_ONLY')
  vin_bin_srcs = vin_bin_srcs_cli_only
  vin_bin_deps = vin_bin_deps_cli_only
  add_global_arguments('-DCLI_ONLY', language : 'cpp')
else

  qtprocessed = qt6_fn.preprocess(moc_headers: 'include/vin/vin_gui.hpp',
                             ui_files: 'src/viz/main_dialog.ui',
                             include_directories: ['include/'])

  vin_bin_srcs = vin_bin_srcs_full + qtprocessed
  vin_bin_deps = vin_bin_deps_full
endif


vin_bin = executable(
  'vin',
  vin_bin_srcs,
  cpp_args: [],
  include_directories: ['include/'],
  dependencies: vin_bin_deps,
  install: true,
  install_dir: bin_install_dir,
  link_args : vin_bin_linker_args
)

##############################
###### QTIO library  #########
##############################
if get_option('BUILD_QTIO')
  
  sources_qtio = ['src/qtio/qt_input.cpp', 'src/qtio/register_nodes.cpp']
  moc_sources_qtio = ['include/qtio/qt_io.hpp']
  # Imageview includes a GUI element so we treat it differently
  if not get_option('CLI_ONLY')
    moc_sources_qtio += ['include/qtio/image_view.hpp']
    sources_qtio += ['src/qtio/image_view.cpp', 'src/qtio/imageview_output.cpp']
  endif

  qtprocessed_io = qt6_fn.preprocess(moc_headers: moc_sources_qtio,
                                      include_directories: ['include/'])

  qtio_lib = shared_library(
    'qtio',
    [sources_qtio, qtprocessed_io],
    cpp_args: ['-fPIC', '-fvisibility=hidden'],
    include_directories: ['include/'],
    dependencies: [qt6multim_dep, qt6widgets_dep, arrow_dep, glog_dep],
    install: true,
    install_dir: vin_lib_dir,
    link_args : vin_bin_linker_args,
  )
endif

##############################
###### QTOP library  #########
##############################
if get_option('BUILD_QTOP')
  qtop_lib = shared_library(
    'qtop',
    ['src/qtio/qt_ops.cpp'],
    cpp_args: ['-fPIC', '-fvisibility=hidden'],
    include_directories: ['include/'],
    dependencies: [qt6gui_dep, arrow_dep, glog_dep],
    install: true,
    install_dir: vin_lib_dir,
  )
endif

##############################
###### Saccade library  #########
##############################
if get_option('BUILD_OPTIMS')
  qtop_lib = shared_library(
    'saccade',
    ['src/saccade_policy/saccade_policy_io.cpp', 'src/saccade_policy/grad_heatmap.cpp'],
    cpp_args: ['-fPIC', '-fvisibility=hidden'],
    include_directories: ['include/'],
    dependencies: [fdag_dep, arrow_dep, glog_dep],
    install: true,
    install_dir: vin_lib_dir,
  )
endif

########################################
####### Lint command (optional) ########
########################################
clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    checks_str = '-clang-diagnostic-*,modernize-*,cppcoreguidelines-*,-readability-*,llvm-header-guard,misc-noexcept-move-constructor,misc-move-const-arg'
    run_target(
        'lint',
        command: [
            clangtidy,
            '-p',
            meson.project_build_root(),
        ] + ['../src/vin/main.cpp', 'vin/vin_dag_manager.cpp', '-checks='+checks_str, '-warnings-as-errors=*'],
    )
endif

# ########################################
# ##### Packaging specifications #########
# ########################################
if build_machine.subsystem() != 'macos'
    arch_id = get_option('ARCH_ID')

    control_cp = fs_fn.copyfile(
        'debian/control',
        install: true,
        install_dir: '../../DEBIAN/',
    )
    deb_package_name = meson.project_name() + '-' + meson.project_version() + '.' + arch_id + '.deb'
    mytarget = custom_target(
        'debpackage',
        # input: 'DEBIAN/control',
        output: deb_package_name,
        command: ['dpkg-deb', '-b', 'install/', '@OUTPUT@'],
    )
endif
