This is currently a very simple CCCR raw file processor for
Tesla EAP images.

Provide it with a raw file and one or two output filenames.
First filename would get a B&W stream with red channel replaced by
averages of surrounding pixels.
If 2nd output filename is specified, extracted red channel data would
be written there (half the original resolution).

The resultant files could be turned into TIFF files like this using
ImageMagick:
convert -depth 16 -size 1280x960 gray:processed_image.raw processed_image.tiff
convert -depth 16 -size 640x480 gray:processed_red.raw processed_red.tiff

Alternatively you can load them directly into Adobe Photoshop.
Parameters:
Channels: 1
Bits per channel: 16

For B&W image resolution is 1280x960
For extracted red image resolution is 640x480

In Adobe Photoshop useful way to get immediately visible image is to go to
Image->Adjustment->HDR toning and then select Method as "Equalize histogram"

Enjoy.
