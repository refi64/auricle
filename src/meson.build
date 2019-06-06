auricle_sources = [
  'main.c',
  'auricle-image-section.c',
  'auricle-music-file.c',
  'auricle-music-row.c',
  'auricle-music-table.c',
  'auricle-notification.c',
  'auricle-options-editor.c',
  'auricle-progress-row.c',
  'auricle-progress-view.c',
  'auricle-renderer.c',
  'auricle-render-options.c',
  'auricle-utils.c',
  'auricle-window.c',
]

auricle_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('gstreamer-1.0'),
  dependency('gstreamer-app-1.0'),
  dependency('gstreamer-video-1.0'),
]

gnome = import('gnome')

auricle_sources += gnome.compile_resources('auricle-resources',
  'auricle.gresource.xml',
  c_name: 'auricle'
)

executable('auricle', auricle_sources,
  dependencies: auricle_deps,
  install: true,
)
