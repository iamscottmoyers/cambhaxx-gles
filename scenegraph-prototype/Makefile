target:
	g++ -g -Wall -pedantic scenegraph.cpp -o scenegraph && ./scenegraph > scenegraph.dot && dot -Tpng scenegraph.dot > scenegraph.png && open scenegraph.png

clean:
	rm -f scenegraph
	rm -f scenegraph.png
	rm -f scenegraph.dot
