# Welcome to CryptograFace!

CryptograFace is an innovative C++ application that combines the robust security of facial biometrics with the convenience
of data encryption and management. Designed with a user-friendly graphical interface, this project offers a seamless
authentication process through facial recognition, ensuring secure access to encrypted text and image files.

Upon logging in, users gain access to their personalized directory, where they can securely browse and manage their encrypted data.
Whether safeguarding sensitive documents or protecting personal drawings, CryptograFace provides a powerful solution for
users seeking both advanced security measures and intuitive file management capabilities.

## Core Features

### Facial Biometric Authentication

CryptograFace incorporates a robust facial recognition system leveraging advanced computer vision techniques. Using OpenCV,
the application captures images of users' faces on dedicated worker threads, maintaining responsiveness on the main GUI
window. Facial descriptors are then extracted using Dlib's powerful deeplearning neural-network models, generating unique
vector representations of facial features. These descriptors are grouped via the Chinese Whispers Clustering algorithm,
ensuring the system's ability to identify and authenticate users accurately.

With security at the forefront of considerations, the threshold for facial descriptor matches is fine-tuned to reduce
false positives. During user login, the system employs reinforced learning techniques to continuously update the dataset
of facial features. This adaptive approach ensures that the system adapts to changes in user appearance and becomes
increasingly reliable over time.

### Hand-Tracked Canvas

In addition to managing text-based data, CryptograFace also offers a unique hand-tracked virtual drawing canvas feature.
Leveraging Python's extensive machine learning libraries, including MediaPipe and PyTorch's pretrained models, this
capability allows users to interact with a digital canvas directly within a live camera feed. A Python interpreter embedded
within the C++ code facilitates real-time hand tracking, enabling precise detection of user finger gestures and positions.

Cryptograface enables users to paint directly on a digital canvas overlaid on the live camera feed. Users can select from
multiple colors and brush sizes to create intricate digital artworks. Moreover, painting progress is automatically saved
across sessions, with each image securely encrypted to ensure data integrity and privacy.

### Secure Encryption

CryptograFace employs Crypto++'s comprehensive suite of encryption functionalities to secure both text-based and image
data using the Advanced Encryption Standard (AES). During encryption, CryptograFace dynamically generates new pairs of
private keys and initialization vectors for each operation. This approach enhances security by preventing potential
vulnerabilities associated with compromised keys. Moreover, the decryption process in CryptograFace includes checksum
verification to detect and prevent corruption of encrypted ciphers. This additional layer of security guarantees the
authenticity of decrypted data, providing users with confidence in the integrity of their stored information.

For example,
```
Lorem ipsum dolor sit amet
```
might get encoded into
```
2E8F0EE35012A35784A55A565C637889CE6FB17E3A07AC8506D7206E0ADAEDFBBF0D5F169E6F935257799F5196
```

### Graphical User Interface

CryptograFace features an intuitive graphical user interface (GUI) built with C++'s Qt framework, Embracing a minimalist
design philosophy that ensures ease of navigation. The architecture of the GUI is optimised with multi-threading capabilities,
ensuring responsiveness even during resource-intensive tasks. Static windows are defined using Qt Modeling Language (QML),
while dynamic components such as file directories are generated dynamically at runtime. The GUI not only handles file
management operations such as creation, deletion, and renaming, but it also integrates user-friendly features such as
pop-up reminders for unsaved content.

![Screenshot 2024-06-29 at 2.52.31 PM.png](..%2F..%2FDesktop%2FScreenshot%202024-06-29%20at%202.52.31%E2%80%AFPM.png)
![Screenshot 2024-06-29 at 3.00.07 PM.png](..%2F..%2FDesktop%2FScreenshot%202024-06-29%20at%203.00.07%E2%80%AFPM.png)
![Screenshot 2024-06-29 at 2.54.47 PM.png](..%2F..%2FDesktop%2FScreenshot%202024-06-29%20at%202.54.47%E2%80%AFPM.png)

## Installation

### Prerequisites

Ensure that you have the following installed
- [C++ 11](https://isocpp.org/get-started) or higher
- [Python 3.9](https://www.python.org/downloads/) or higher
- [OpenCV](https://opencv.org/releases/)
- [Dlibs](http://dlib.net/compile.html)
- [MediaPipe](https://github.com/google-ai-edge/mediapipe)
- [Crypto++](https://cryptopp.com/downloads.html)
- [Qt](https://doc.qt.io/qt-6/get-and-install-qt.html)

### Setup

Clone this repo with the command
```
git clone https://github.com/alfaloo/CryptograFace.git
```

Run the `CMakeLists.txt` file.

You are now ready to go!


## License

This project is under the [MIT license](https://opensource.org/license/mit).


## Acknowledgement

Special thanks to the creators of OpenCV, Dlibs, Mediapipe, Crpyto++, and Qt for their amazing libraries/frameworks,
without which, the completion of this project would not be possible.

Additionally, I would like to shoutout:
- [@packetcode](https://github.com/packetcode), for their [face-recognition-opencv](https://github.com/packetcode/face-recognition-opencv/tree/master) project, from which I have taken great inspiration.
- [@MohamedAlaouiMhamdi](https://github.com/MohamedAlaouiMhamdi), for his [AI_virtual_Painter](https://github.com/MohamedAlaouiMhamdi/AI_virtual_Painter/tree/main) project, from which I have taken great inspiration.
- [@serengil](https://github.com/serengil), for his [Fine Tuning The Threshold in Face Recognition](https://sefiks.com/2020/05/22/fine-tuning-the-threshold-in-face-recognition/) analysis, from which I have taken great inspiration.