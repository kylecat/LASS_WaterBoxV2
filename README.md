# LASS_WaterBoxV2
 ### LASS IS YOURS

1. 功能介紹 

   水盒子是一台完全開源，可讓Maker視需求自行更換感測項目的水質感測設備。水盒子採用臺灣聯發科設計製造的LinkIt7697開發版作為微控制器(MCU)，以免費開源軟體ArduinoIDE作為開發環境，同時將常用的Arduino模組集成設計在一塊PCB電路板上，這些功能可經由開源的Library作為驅動，目前的測試程式碼跟主要運作程式碼也是以MIT授權的方式開放所有人使用，程式碼相關內容請參考後續章節韌體說明。

    目前水盒子可量測項目為水溫、導電度(Electrical Conductivity, EC)及酸鹼度(pH)等自然水質基本項目，資料可以*.CSV格式(或其他格式)儲存於MircoSD卡內，經WiFi上傳至雲端(LASS或ThingSpeak)或以LoRa/BLE 通訊方式傳輸到其他裝置上；由於自然水體內水質情況會隨著不同地點而有不同的干擾，即使為無工業污染的水體也會有生物膜附著在電極表面造成感測電極干擾，因此建議每周維護檢查電極偏差的情況，確保所量測水質資料具參考價值。

    ***水盒子是為了擴增LASS環境感測器網路系統的感測領域而設計的開源專案，單一一台水盒子只能知道一池水的水質狀況，而經由公民科學參與大量布建則可揭露環境/污染變化的分布及趨勢；因此，希望使用者能上傳/分享水盒子的感測資料，一同為我們所生活這塊土地努力。***
