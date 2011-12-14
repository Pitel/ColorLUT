Barevná korekce (color grading) pomocí LUT
==========================================

Projekt do předmětu GMU

Použití
-------
Spuštění programu s parametry: `colorLUT <input> [-s n] <LUTs>…`

* `input` – vstupní soubor (obrázek, .avi, .mpg nebo .mpeg)
* `-s n` – n velikost lookup tabulky (vychozí hodnota 16), nastavuje se pro všechny následující LUT
* `LUTs` – obrázky lookup tabulek (tabulka musí být v levém horním rohu)
* Přepínání mezi zadanými lookup tabulkami pomocí mezerníku

Příklady spuštění:

* `colorLUT input.png -s 2 lut1.png lut2.png -s 8 lut3.png lut4.png`
* `colorLUT input.avi lut.jpg lut1.jpg`

Poznámka: Pro překlad v CVT FIT použijte příkaz `make cvt`

Zdroje
------
* [GPU Gems2, Chapter 24. Using Lookup Tables to Accelerate Color Transformations](http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter24.html)
* [Google I/O 2011: WebGL Techniques and Performance](http://youtu.be/rfQ8rKGTVlg?t=24m30s)
* [UDN - Three - ColorGrading](http://udn.epicgames.com/Three/ColorGrading.html)
* [An ffmpeg and SDL Tutorial](http://dranger.com/ffmpeg/ffmpeg.html)
