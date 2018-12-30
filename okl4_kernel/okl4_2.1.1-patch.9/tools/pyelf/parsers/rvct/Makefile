RVCT_LDS_FILE=rvct_lds.lds
OUTPUTFILE=.
DATADIR="test/data"

#DEBUG=-traceParser

all:	
	runantlr -o $(OUTPUTFILE) $(DEBUG) rvct_ld_l.g 
	runantlr -o $(OUTPUTFILE) $(DEBUG) rvct_ld_p.g 
	-touch $(OUTPUTFILE)/__init__.py


#run generated parser python script
test_p:	all
	 python $(OUTPUTFILE)/rvct_ld_p.py < $(DATADIR)/$(RVCT_LDS_FILE)

#run generated lexer python script
test_l:	all
	 python $(OUTPUTFILE)/rvct_ld_l.py < $(DATADIR)/$(RVCT_LDS_FILE)


clean:
	-rm -f rvct_ld_l.py* rvct_ld_p.py* RVCT_LDTokenTypes.txt rvct_ld_pTokenTypes.txt rvct_ld_wTokenTypes.txt rvct_ld_l.txt
