# mruby-scintilla-termbox2

mruby binding for [scintilla-termbox2](https://github.com/masahino/scintilla-termbox2).

## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem github: 'masahino/mruby-scintilla-termbox2'
end
```

## Usage

```ruby
editor = Scintilla::ScintillaTermbox2.new
editor.SCI_SETTEXT('hello')
editor.refresh
```

## License
under the MIT License:
- see LICENSE file
