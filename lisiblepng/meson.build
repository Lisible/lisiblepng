lisiblepng_incdir = include_directories('src/')
lisiblepng_lib = library('lisiblepng', 'src/lisiblepng.c', 'src/deflate.c', 'src/bitstream.c')
lisiblepng_dep = declare_dependency(include_directories: lisiblepng_incdir, link_with: [lisiblepng_lib])
