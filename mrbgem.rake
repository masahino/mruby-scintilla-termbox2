MRuby::Gem::Specification.new('mruby-scintilla-termbox2') do |spec|
  spec.license = 'MIT'
  spec.authors = 'masahino'
  spec.add_dependency 'mruby-scintilla-base', github: 'masahino/mruby-scintilla-base'
  spec.add_dependency 'mruby-termbox2', github: 'masahino/mruby-termbox2'
  spec.version = '5.6.6'

  def spec.download_scintilla
    return if @scintilla_download_configured

    @scintilla_download_configured = true
    require 'open-uri'
    scintilla_ver = '566'
    scintilla_url = "https://scintilla.org/scintilla#{scintilla_ver}.tgz"
    scintilla_termbox2_url = 'https://github.com/masahino/scintilla-termbox2'
    scintilla_build_root = "#{build_dir}/scintilla/"
    scintilla_dir = "#{scintilla_build_root}/scintilla"
    scintilla_a = "#{scintilla_dir}/bin/scintilla.a"
    scintilla_termbox2_dir = "#{scintilla_dir}/termbox2"
    scintilla_h = "#{scintilla_dir}/include/Scintilla.h"
    scintilla_termbox2_h = "#{scintilla_termbox2_dir}/ScintillaTermbox2.h"
    flags = ''

    file scintilla_h do
      URI.open(scintilla_url, open_timeout: 10, read_timeout: 30) do |http|
        scintilla_tar = http.read
        FileUtils.mkdir_p scintilla_build_root
        IO.popen("tar xfz - -C #{filename scintilla_build_root}", 'wb') do |f|
          f.write scintilla_tar
        end
        raise "tar failed: #{scintilla_url} (#{$?.exitstatus})" unless $?.success?
      end
      raise "#{scintilla_h} not produced" unless File.exist?(scintilla_h)
    end

    # scintilla-termbox2 have no tags or releases, so the latest
    # default branch is cloned. rm_rf clears any partial checkout from a prior
    # failed run; sh (argv form) uses no shell and raises on a non-zero exit.
    file scintilla_termbox2_h => scintilla_h do
      rm_rf scintilla_termbox2_dir
      sh 'git', 'clone', '--depth', '1', scintilla_termbox2_url, scintilla_termbox2_dir
    end

    file scintilla_a => [scintilla_h, scintilla_termbox2_h] do
      sh %((cd #{scintilla_termbox2_dir} && make -j4 CXX=#{build.cxx.command} AR=#{build.archiver.command} EXTRA_FLAGS="#{flags}"))
    end

    task :mruby_scintilla_termbox2_compile_option do
      linker.flags_before_libraries << scintilla_a

      linker.libraries << 'stdc++'
      linker.libraries << 'pthread'
      [cc, cxx, objc, mruby.cc, mruby.cxx, mruby.objc].each do |compiler|
        compiler.include_paths << "#{scintilla_dir}/include"
        compiler.include_paths << "#{scintilla_dir}/src"
        compiler.include_paths << scintilla_termbox2_dir
        compiler.include_paths << "#{scintilla_termbox2_dir}/vendor/termbox2"
      end
    end
    file "#{dir}/src/scintilla_termbox2.c" => [:mruby_scintilla_termbox2_compile_option, scintilla_a]
  end

  spec.download_scintilla
end
