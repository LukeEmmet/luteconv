Luteconv - Convert between lute tablature file formats
======================================================

There is a variety of lute tablature editing and formatting software available, with
a corresponding variety of file formats used to store the lute tablature.  Luteconv
converts between some of these different formats.

The original motivation for this project was so that pieces from the vast library
of lute tablature available in Fronimo .ft3, Fantango .jtz, Tab .tab and TabCode .tc
formats on the net[1,6,7,8] could be imported into MuseScore as MusicXML.
The currently supported file formats are:

Fandango jtxml, jtz
-------------------
Fandango[1] is a commercial Windows lute tablature editor written by Alain Veylit. Its file format,
jtxml, is a closed, proprietary XML format; jtz is zip compressed jtxml.  However, Luke Emmet[3] has
reverse engineered the format for his program LuteScribe so that some of the data can be extracted.
I am grateful for his work on this.

Fronimo ft3
-----------
Fronimo[2] is a commercial Windows lute tablature editor written by Francesco Tribioli. Its file format,
ft3, is a closed, proprietary binary format.  However, Luke Emmet[3] has reverse engineered the format for his program LuteScribe
so that some the data can be extracted.  I am grateful for his work on this.

MusicXML musicxml, mxl
----------------------
MusicXML[4] is an music interchange file format and is supported by many music editing programs
e.g. MuseScore, Sibelius, Finale etc.  Its file format, musicxml, is an open,
documented XML format; mxl is a zip compressed musicxml. Although designed primarily for mensural notation
MusicXML does support lute (and guitar) tablature.  MusicXML is well documented.

Tab tab
-------
Tab[5] is a lute tablature type setting program written by Wayne Cripps.  Its file format, tab, is a text file designed for
user input.  Tab is the lingua franca of tablature file formats and is often supported as an import format by other lute tablature
editing software.  Tab is well documented and its source code is published.

TabCode tc
----------
TabCode[6] devised by Tim Crawford.  Used by ECOLM - An Electronic Corpus of Lute Music, Goldsmith's University of London.
Where there is a large searchable database of lute pieces.

Supported source formats
------------------------
ft3, jtxml, jtz, musicxml, mxl, tab and tc.

Supported destination formats
-----------------------------
musicxml, mxl, tab and tc.

Supporting proprietary destination formats (jtxml, jtz, ft3) is not possible as this would require
complete knowledge of their structure and semantics, which is not available.  Whereas they can be used as
source formats as enough information has been gleaned by reverse engineering to give at least the notes,
rhythm and bars.

Converting a file to its own format is supported, although some information may be lost.  However, it is useful
for converting between different tablature types: french, italian, spanish.

Limitations
-----------
* Solo instrument.
* No text underlying tablature for songs etc.
* Conversions are "best effort" some data (specific fonts, layout, special symbols) may be lost. 

Platform
--------
luteconv is implemented for Linux (openSUSE) and although it should work on other Linux distributions or
possibly the Windows Subsystem for Linux (WSL) it has not been tested on other platforms.

Usage
-----
Usage: luteconv [options ...] source-file [destination-file]

| option | function |
| ------ | -------- |
| --help                      | Show help                      |
| --version                   | Show version                   |
| --output <destination-file> | Set destination-file           |
| --Srctabtype <tabtype>      | Set source tablature type      |
| --Dsttabtype <tabtype>      | Set destination tablature type |
| --srcformat <format>        | Set source format              |
| --dstformat <format>        | Set destination format         |
| --tuning <tuning>           | Set tuning for all courses     |
| --7tuning <tuning>          | Set tuning from 7th course     |
| --index <index>             | Set section index              |
| --Verbose                   | Set verbose output             |

Options may use long or short syntax: --7tuning=D2 or -7D2

The destination-file can be specified either using the --output option or as the 2nd positional parameter,
this conforms to the GNU Standards for Command Line Interfaces[9].
 
format = "ft3" | "jtxml" | "jtz" | "musicxml" | "mxl" | "tab" | "tc"
  
   if a file format is not specified then the filetype is used.
         
tabtype = "french" | "italian" | "spanish"

   The source tablature type is usually deduced from the source-file.  However, for tab
   files it is sometimes necessary to distinguish between italian and spanish tablatures.
   The default destination tablature type is french.
            
tuning = Courses in scientific pitch notation[10], in increasing course number.
  
   Use the tuning specfed by option --tuning, if given.  
   else use tuning specified in the source file, if any.
   else based on number of courses used:  
      = 8   "G4 D4 A3 F3 C3 G2 F2 D2"  
      <= 10 "G4 D4 A3 F3 C3 G2 F2 Eb2 D2 C2"  
      >= 11 "F4 D4 A3 F3 D3 A2 G2 F2 E2 D2 C2 B1 A1"  
      Option --7tuning will then modify the above
         
Where the source format allows more that one piece per file the --index option selects the
desired piece, counting from 0.  Default 0.

Examples
--------

Convert tab to musicxml

    luteconv Kapsberger-Gagliarda5a.tab Kapsberger-Gagliarda5a.musicxml

Convert Luis de Milan (spanish tablature) tab to mxl 

	luteconv --Srctabtype=spanish Milan_Fantasia_10.tab Milan_Fantasia_10.mxl
	
Convert 7 course piece with 7th course tuned to D

	luteconv --7tuning=D2 Loath.tab Loath.mxl
	
Convert 2nd piece from a Fandango collection to tab (index counts from 0)

	luteconv --index=1 Willoughby.jtz Fantacy.tab

Build
-----

The implementation is for Linux using cmake tested on openSUSE 15.2, but
I expect any modern Linux or BSD distribution could be used. In a
suitable workspace directory:

    git clone https://bayleaf@bitbucket.org/bayleaf/luteconv.git
    mkdir build
    cd build
    cmake ../luteconv
    make
    make test
    make package

The executable will be in bin.  The .rpm and .deb packages will be in the build directory.

TODO
----
1 Other lute tablature file formats to consider:

* [abctab](http://www.lautengesellschaft.de/cdmm/)
* [TabXML](https://webspace.science.uu.nl/~wieri103/tabxml/)
* [BeierTab](https://www.musico.it/beier-software/beiertab-2/)
* [LuteScribe](https://www.orlando-lutes.com/pages/lutescribe)

2 More research into Fronimo ft3 format.

3 More research into Fandango jtxml format.

References
----------
1.  [Fandango](http://fandango.musickshandmade.com/pages/fandango)
2.  [Fronimo](https://sites.google.com/view/fronimo/home)
3.  [LuteScribe](https://www.orlando-lutes.com/pages/lutescribe)
4.  [MusicXML](https://www.musicxml.com/)
5.  [Tab](https://www.cs.dartmouth.edu/~wbc/lute/AboutTab.html)
6.  [TabCode](http://doc.gold.ac.uk/isms/ecolm/?page=TabCode)
7.  [Accessible Lute Music](http://www.gerbode.net/)
8.  [Tab Tablature](https://www.cs.dartmouth.edu/~wbc/tab-serv/tab-serv.cgi)
9.  [GNU](https://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html)
10.  [Scientific Pitch Notation](https://en.wikipedia.org/wiki/Scientific_pitch_notation)

