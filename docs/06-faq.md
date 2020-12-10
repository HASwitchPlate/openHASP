
#### Q: HASP Settings

A:

#### Q: Is there a file browser built-in?

*A:* There is no native file browser included yet, as this currently is low on the priority list.

However, you can upload the `edit.htm.gz` (3kB) file to the SPIFFS partition from the ESP32 FSBrowser repository.
Download it from: https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/FSBrowser/data/edit.htm.gz

When the `edit.htm.gz` file is present on Spiffs you will see an additional File Browser button on the Main Webpage:
![HTTP configuration](assets/images/hasp/faq_file_browser.png "File Browser")

Using that webpage, you can right-click and delete files:
![HTTP configuration](assets/images/hasp/faq_file_delete.png "Delete file")
