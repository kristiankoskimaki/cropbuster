Cropbuster 1.0
--------------

Source code: https://github.com/kristiankoskimaki/cropbuster  
Windows exe: https://github.com/kristiankoskimaki/cropbuster/releases


Cropbuster can quickly scan through a large collection of jpegs and display only those images that have a border (frame).
Such images can include screen captures, image macros (memes) or digital art. Cropbuster can detect where the actual image
inside the border is and allows you to save this image with the border removed.


Features:
 - Simple to use graphical UI
 - 64-bit, multithreaded program.
 - High detection rate for images with borders, low false positive rate
 - Uses OpenCV computer vision library
 - Open source, GPL v3 license


<!---
Usage:  
After starting Cropbuster you must enter the folders where to scan for images. Folders can be added by typing them in,
dragging and dropping a folder onto the window or using the folder browser button next to the input box.
All folders must be separated by a semicolon ( ; ).

Search is started by pressing the "Find images" button and all jpeg files in selected folders are scanned. Other formats, such as
PNG are not supported by default (see below) as they are usually not a digital photograph with a drawn border, which is what Cropbuster was made to find.
Filenames of found images appear in the list on the right side of the program. Select any one of them to display the image on the left side of the program.
There will be a clickable link in the lower right corner that opens the image in your file manager. Image resolution and selection box dimensions are displayed below.

The selection box
Cropbuster will try to detect where the border ends and the image begins and mark it with a green rectangle. If the detection is wrong, you can adjust it with the eight arrow buttons around the image. The zoom function can be useful here, as you will see the dividing line pixel perfect.

If you are satisified with the border detection and wish to save the image with the border removed, click on the Save as... button. The filename defaults to overwriting the image, but you can save the image with any name in any folder.


Settings:
Thread limiter      The spin box in the lower left corner sets the amount of CPU threads Cropbuster uses.
                    Values range from 1 to as many threads your CPU has. Lower the value if you do not want Cropbuster to use the CPU entirely.
JPG quality         This spin box sets the quality of saved jpeg images. Values range from 1 (very poor) to 100 (very large size). 80 is default.
Black border only   This setting switches between finding images with a black border or images with any color border.
                    Black border color is common and this setting finds almost any of them.
					Unticking this setting will search for images with any color (white, green...) border.
					This setting may result in finding more false positives, such as scanned images or very bright photos.
Zoom button			This button is located in the top left corner. Cropbuster will show the selected image either as a
                    scaled version that fits entirely in the window, or zoomed in to its actual size.

The file settings.ini contains additional settings that control the border detection. The default settings are quite good for finding most images,
so changing them is not necessary. You can make Cropbuster read other image formats, such as PNG, by modifying the IMAGE_FORMATS setting.
--->


![](https://user-images.githubusercontent.com/46446783/147774260-7a32c1e7-5db5-4536-b993-eb367864216c.png)
Image with border found by Cropbuster. Top edge manually adjusted, left and bottom edges as automatically detected.


Cropbuster Copyright (C) 2021 Kristian Koskimäki  
Cropbuster is a free software distributed under the GNU GPL.  
Read LICENSE.txt for more information.
