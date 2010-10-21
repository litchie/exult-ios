CLASSPATH = xml-apis.jar:xercesImpl.jar:xalan.jar
JAVA = java
INPUT = readme.xml faq.xml
CLASS = org.apache.xalan.xslt.Process

HTML_OBJS := $(patsubst %.xml,%.html,$(wildcard *.xml))
TEXT_OBJS := $(patsubst %.xml,%.txt,$(wildcard *.xml))
PHP_OBJS := $(patsubst %.xml,%.dat,$(wildcard *.xml))

all: html text php ReadMe.html duplicate phpfix

# Rules for transforming the XML source
.xml.html: html.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL html.xsl -OUT $@
.xml.txt: text.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL text.xsl -OUT $@
.xml.dat: php.xsl
	$(JAVA) -classpath $(CLASSPATH) $(CLASS) -IN $< -XSL php.xsl -OUT $@
.SUFFIXES: .xml .html .txt .dat

# rule for renaming docs.html to ReadMe.html
ReadMe.html: docs.html
	mv docs.html ReadMe.html

#rule for copying faq.txt, docs.txt to Faq and ReadMe
duplicate: faq.txt docs.txt
	cp faq.txt FAQ
	cp docs.txt README

# rule for stripping the unneded <test> tag from the PHP files
phpfix: docs.dat faq.dat exult_studio.dat
	sed -i -e 's@<test xmlns="http://www.w3.org/1999/xhtml">@@; s@</test>@@' *.dat

# remove all generated files
clean:
	-rm -f *.html *.txt *.dat FAQ README

html: $(HTML_OBJS)
text: $(TEXT_OBJS)
php: $(PHP_OBJS)

.PHONY: all html text php clean
