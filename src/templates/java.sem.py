#sem:name: Java applet
#sem:tip: Generates a web applet in java (requires java)
#sem:picture

# Thomas Nagy, 2007-2012

import os, shutil, time, sys

# TODO for the moment it does not work

outdir = sembind.get_var('outdir')+'/'+sembind.get_var('pname')

try:
	tm = time.strftime('.%d-%b-%y--%H-%M-%S', time.gmtime(os.stat(outdir).st_mtime))
	os.rename(outdir, outdir+tm)
except OSError:
	pass

try:
	os.makedirs(outdir)
except OSError:
	sys.stderr.write("Cannot create folder " + outdir)

for x in "Makefile main.html Kadapplet.java Mapview.java".split():
	shutil.copy2(template_dir() + '/java/'+x, outdir)

code = []
def out(s):
	global code
	code.append(s)

def comp(a, b, c, d, e):
	s = "\nif (x > %d && y > %d && x < %d && y < %d) { return %s; }\n" % (a, b, c, d, e)
	out(s)

out("/** Generated by Semantik */\n\n")
out("public class TextCoord\n{\n")
out("public static String coordToText(double x, double y) {\n")

# for ...
#        comp()

out("return \"\";\n")
out("}\n}\n")

file = open(outdir+'/TextCoord.java', 'w')
try:
	file.write(''.join(code))
finally:
	file.close()


