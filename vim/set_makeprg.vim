augroup SetMakeprg
	autocmd!
	autocmd BufEnter */dirname/* let &makeprg = "make -C " . substitute(expand("<afile>:p"), "^\\(\\f\\+dirname\\)/\\f*", "\\1", "") . " "
	autocmd BufLeave * set makeprg&
augroup END
