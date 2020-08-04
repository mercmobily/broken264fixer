# Fix broken 264 files from cheap security cameras

The market is full of cheap security cameras;  they have one problem: the files they generate, which are allegedly 264 files, are impossible to play on any player -- they will only play on proprietary software.

This page is meant to be a one-stop shop to convert those files into something playable, in Linux, Windows and Mac. (Note: Windows an Mac are coming), thanks to [the work of Ralph Spitzner and Ian Macallan](https://www.spitzner.org/kkmoon.html) (please note that if you found this page useful you should _strongly_ consider clicking on the "Donate" button on this page).

Please note that I didn't code anything: all I did was package the necessary commands in a bundle, so that normal users can actually view the videos provided by those cameras.

## Linux

For Linux, download the file [broken264fixer](https://mercmobily.github.io/broken264fixer/broken264fixer), make it executable:

````
chmod +x broken264fixer
````

And simply run it:

````
broken264fixer inputFile.264
````

The result will be `inputFile.264.mkv`, which is fully playable.

## Technical notes

The "264" files created by those cameras are not actually MP4 files -- instead, they are files that contain interleaved blocks of video frames and blocks of audio samples, seperated by the HXAV/HXAF markers (in Ralph's own words). This is likely due to the fact that the camera makers don't want to pay licensing fees to create proper MP4 files.
