cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: true)

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DLPNG_DEBUG_LOG', language: ['c'])
  add_project_arguments('-DLPNG_COMPUTE_CRC', language: ['c'])
endif

lisiblestd_dep = dependency('lisiblestd')
lisiblepng_incdir = include_directories('src/')
lisiblepng_lib = library('lisiblepng', 'src/lisiblepng.c', 'src/lisiblepng/deflate.c', 'src/lisiblepng/bitstream.c', dependencies: [m_dep, lisiblestd_dep])
lisiblepng_dep = declare_dependency(include_directories: lisiblepng_incdir, link_with: [lisiblepng_lib], dependencies: [m_dep, lisiblestd_dep])
