let SessionLoad = 1
if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
imap <F3> \tf
imap <F4> \tp
map! <F1> <F1>
map! <S-Insert> <MiddleMouse>
nmap  :b 
vnoremap  "ad
nnoremap  "aP
vnoremap  "aP
vnoremap  "ay
vnoremap Q gq
nnoremap Q gqap
noremap Y y$
vmap [% [%m'gv``
nmap <silent> \bundo <Plug>BufKillBundo
nmap <silent> \!bw <Plug>BufKillBwBang
nmap <silent> \bw <Plug>BufKillBw
nmap <silent> \!bd <Plug>BufKillBdBang
nmap <silent> \bd <Plug>BufKillBd
nmap <silent> \!bun <Plug>BufKillBunBang
nmap <silent> \bun <Plug>BufKillBun
nnoremap \th :set invhls hls?
nnoremap \tl :set invlist list?
nnoremap \tf :if &fo =~ 't' | set fo-=t | else | set fo+=t | endif | set fo?
nnoremap \tp :set invpaste paste?
vmap ]% ]%m'gv``
vmap a% [%v]%
nmap gx <Plug>NetrwBrowseX
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetBrowseX(expand("<cWORD>"),0)
nmap <F2> \tl
nmap <F3> \tf
nmap <F4> \tp
omap <F1> <F1>
vmap <F1> <F1>
nnoremap <F1> :help 
nnoremap <S-F6> W
nnoremap <F6> w
noremap <Del> 2
noremap <Insert> 2
nmap <F8> :mksession!
map <S-Insert> <MiddleMouse>
iabbr lfpg Llanfairpwllgwyngyllgogerychwyrndrobwllllantysiliogogogoch
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set background=dark
set backspace=eol,start,indent
set comments=://,b:#,:%,:XCOMM,n:>,fb:-,s:/*,mb:**,ex:*/,fb:*,b:\",n::
set dictionary=~/.dictionary
set display=lastline
set gdefault
set guicursor=a:blinkon0
set guifont=Neep\ Alt\ 10
set guioptions=aegirLt
set helplang=en
set hidden
set history=50
set hlsearch
set ignorecase
set iminsert=0
set imsearch=0
set incsearch
set listchars=eol:$,tab:»·
set mouse=a
set pastetoggle=<F4>
set printoptions=paper:a4
set ruler
set runtimepath=~/.vim,/var/lib/vim/addons,/usr/share/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim70,/usr/share/vim/vimfiles/after,/usr/share/vim/addons/after,/var/lib/vim/addons/after,~/.vim/after
set shiftwidth=4
set shortmess=filnxtToOr
set showcmd
set showmatch
set smartcase
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set tabstop=4
set termencoding=utf-8
set viminfo=/10,'10,r/mnt/zip,r/mnt/floppy,f0,h,\"100
set visualbell
set whichwrap=h,l,b,s,<,>,~,[,]
set wildignore=*.pyc
set wildmenu
set wildmode=longest,full
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/src/magpie-parsers
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +54 dump/cplusplus.py
badd +82 src/magpieparsers/cplusplus/parser.py
badd +556 src/magpieparsers/cplusplus/astgen.py
badd +107 src/magpieparsers/cplusplus/astexpr.py
badd +1731 src/magpieparsers/cplusplus/CPP_parser.g
badd +5 node_types.txt
badd +88 src/magpieparsers/corba3/astgen.py
badd +631 src/magpieparsers/parser_common.py
badd +68 src/magpieparsers/types/evaluator.py
badd +92 src/magpieparsers/types/builtin.py
badd +99 src/magpieparsers/types/arch_sizes.py
badd +0 src/magpieparsers/types/infogripper.py
args dump/cplusplus.py
edit src/magpieparsers/types/builtin.py
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
exe 'vert 1resize ' . ((&columns * 104 + 104) / 209)
exe 'vert 2resize ' . ((&columns * 104 + 104) / 209)
argglobal
setlocal noarabic
setlocal autoindent
setlocal autoread
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal comments=://,b:#,:XCOMM,n:>,fb:-,s:/*,mb:**,ex:*/,fb:*,b:\",n::
setlocal commentstring=#%s
setlocal complete=.,w,b,u,t,i
setlocal completefunc=
setlocal nocopyindent
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != 'python'
setlocal filetype=python
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=0
setlocal imsearch=0
setlocal include=^s*\\(from\\|import\\)
setlocal includeexpr=substitute(v:fname,'\\.','/','g')
setlocal indentexpr=
setlocal indentkeys=0{,0},:,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keymap=
setlocal keywordprg=
set linebreak
setlocal linebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
setlocal nonumber
setlocal numberwidth=4
setlocal omnifunc=pythoncomplete#Complete
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en
setlocal statusline=
setlocal suffixesadd=.py
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'python'
setlocal syntax=python
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal nowinfixheight
setlocal nowinfixwidth
setlocal wrap
setlocal wrapmargin=0
silent! normal! zE
2,77fold
2
normal zc
let s:l = 150 - ((29 * winheight(0) + 31) / 63)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
150
normal! 028l
wincmd w
argglobal
edit src/magpieparsers/types/infogripper.py
setlocal noarabic
setlocal autoindent
setlocal autoread
setlocal balloonexpr=
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal nocindent
setlocal cinkeys=0{,0},0),:,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal comments=://,b:#,:XCOMM,n:>,fb:-,s:/*,mb:**,ex:*/,fb:*,b:\",n::
setlocal commentstring=#%s
setlocal complete=.,w,b,u,t,i
setlocal completefunc=
setlocal nocopyindent
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal noexpandtab
if &filetype != 'python'
setlocal filetype=python
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=tcq
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=0
setlocal imsearch=0
setlocal include=^s*\\(from\\|import\\)
setlocal includeexpr=substitute(v:fname,'\\.','/','g')
setlocal indentexpr=
setlocal indentkeys=0{,0},:,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keymap=
setlocal keywordprg=
set linebreak
setlocal linebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
setlocal nonumber
setlocal numberwidth=4
setlocal omnifunc=pythoncomplete#Complete
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=4
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=0
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en
setlocal statusline=
setlocal suffixesadd=.py
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'python'
setlocal syntax=python
endif
setlocal tabstop=4
setlocal tags=
setlocal textwidth=0
setlocal thesaurus=
setlocal nowinfixheight
setlocal nowinfixwidth
setlocal wrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 2 - ((1 * winheight(0) + 31) / 63)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
2
normal! 0
wincmd w
exe 'vert 1resize ' . ((&columns * 104 + 104) / 209)
exe 'vert 2resize ' . ((&columns * 104 + 104) / 209)
tabnext 1
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToOr
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . s:sx
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
