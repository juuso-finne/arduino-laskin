# Arduino-laskin
Tämä kansio sisältää lähdekoodin Arduino Nano -pohjaiselle laskimelle. Laskin tarvitsee Arduinon lisäksi LCD-näytön ja 4x4-matriisinäppäimistön.
Näyttö ja näppäimistö tulee kytkeä seuraavan pinnikaavion mukaisesti tai muuttaa kyseiset pinnit lähdekoodiin:  

LCD -> Arduino  
  
RS -> A5  
enable -> A4  
d4 -> A3  
d5 -> A2  
d6 -> A1  
d7 -> A0  

Näppäimistö -> Arduino  
  
Row 0 -> 2  
Row 1 -> 3  
Row 2 -> 4  
Row 3 -> 5  
  
Col 0 -> 6  
Col 1 -> 7  
Col 2 -> 8  
Col 3 -> 9  
