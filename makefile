.SILENT:

COMP = g++ -Wall -m64
SQL = -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
QT = -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux

all: LeMaraicher CreationBD

LeMaraicher:	moc_windowclient.o windowclient.o mainclient.o LibSockets.o
		echo compilation de LeMaraicher
		g++ -Wno-unused-parameter -o LeMaraicher mainclient.o windowclient.o moc_windowclient.o LibSockets.o  /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread

mainclient.o:	ClientQt/mainclient.cpp
				echo compilation de mainclient
				$(COMP) $(QT) ClientQt/mainclient.cpp -c

windowclient.o:	ClientQt/windowclient.cpp
				echo compilation de windowclient
				$(COMP) $(QT) ClientQt/windowclient.cpp -c

moc_windowclient.o: ClientQt/moc_windowclient.cpp
				echo compilation de moc_windowclient
				$(COMP) $(QT) ClientQt/moc_windowclient.cpp -c

LibSockets.o: lib/LibSockets.cpp
				echo compilation de LibSockets
				$(COMP) lib/LibSockets.cpp -c

CreationBD:	BD_Maraicher/CreationBD.cpp
			echo compilation de CreationBD
			$(COMP) BD_Maraicher/CreationBD.cpp $(SQL) -o CreationBD

clean:
	rm -f *.o LeMaraicher
