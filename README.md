# Mikrodenetleyici Tabanlı Oyun Makinesi
Projemizin amacı çeşitli araçlar kullanarak mikrodenetleyici tabanlı bir oyun makinesi geliştirmektir. 
Proje Arduino IDE ve Proteus programları kullanılarak geliştirilmiştir. 

Projemizde oyuncu, bir fiziksel palet kontrol cihazını kullanarak bir topu yansıtarak OLED ekrandaki tuğlaları kırmaya çalışacaktır. Oyuncu, fiziksel bir "palet kontrol cihazı" kullanacaktır ve bu cihaz, potansiyometre ile kontrol edilecektir. Oyunumuz başlatıldığında ilk olarak OLED ekranımızda Başlat ve Çıkış seçeneklerinin olduğu bir menüyle başlar. Bu seçenekler Proteus’taki devremizde yer alan 3 buton aracılığıyla seçilebilir. Bir buton aşağı, bir buton yukarı ve diğer buton seçim butonudur. Oyuncu Başlat seçeneğiniseçtiğinde Oyun Başlıyor… yazısı gösterildikten sonra level 1 başlar. Çıkış seçeneğine basıldığında “Oyunumuza gösterdiğiniz ilgi için teşekkürler.” yazısı gösterilip program sonlanır. Oyun başladığında oyuncu potansiyometre yardımıyla top ile tuğlaları kırmaya başlar. Her kırılan tuğla için skor 1 puan artar. Her tuğla kırıldığında yüzde 10 şans ile bir obje düşürecek şekilde ayarlanmıştır. Oyuncu bu düşen objeyi aldığında ek can elde eder. Eğer sönük led varsa tekrar yanmaya başlar. Oyuncu level 1 de yer alan tüm tuğlaları kırdığında 3 saniye kadar bekletilip level 2 ye geçer. Her level için farklı tuğla düzeni kurulmuştur. Oyuncu yanana kadar oyun bu şekilde devam eder. Oyun sonlandığında Oyunu kaybettiniz ve Skor yazar ve 5 saniye gösterilir. Ardından ana menüye döner. 

![Ekran görüntüsü 2024-04-03 123153](https://github.com/caglagok/Tugla_Kirma_Oyunu-Ping_Pong/assets/114026286/cba5fdb6-4043-47ee-a660-d1ed08e9cded)

Detaylı açıklamayı raporda bulabilirsiniz.
