// Подключаем библиотеки
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Button.h"
#include "FFT.h"
#include <string>
#include "tinyfiledialogs.h" // работа с окнами
#include <SFML/System/Time.hpp>
#include <sndfile.h> //работа с аудиофайлами
#include "SliderSFML.h"


float phaseShift = 147.9; // фазовый сдвиг
// инициализируем функции
void FFTAnalysis(double* AVal, double* FTvl, int Nvl, int Nft);
void ref_vol(float val);
void img_vol(float val);
void spect_w(float val);
void audio_size(float val);
void phase(float val);
void encrypt();
void load_ref();
void create_ref();
void selectRef(const std::string& selection);
void load_image();
void selectImage(const std::string& selection);
void processSound(const std::vector<float>& floatData, int chs);
#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288
#endif // !M_PI

// объявляем переменные
int spectX = 120;
int spectY = 80;
int vol1X = 150;
int vol2X = 340;
int volSize = 150;
int imgWidth;
std::vector<float> ref; // вектор для хранения обработанных аудиоданных
std::vector<float> img; // Вектор для хранения данных изображения
float refAmpl = 1.0, imgAmpl = 1.0;
float spectW = 370;
bool updFlag = true; // Флаг, указывающий на обновление данных
bool saveFlag = false;
float phaseVal = 0;
int framerate = 44100; // Примерная частота дискретизации для аудио 
float audioSize = 5;
bool createFlag = false;
sf::RenderTexture pg; 
sf::Image imageCrypt; // Исходное изображение
std::string refPath = "", imagePath = ""; //пути к аудиофайлу и изображению
sf::Image resizedImage; // Измененное изображение
sf::VertexArray lines(sf::Lines, 300);
sf::VertexArray lines1(sf::Lines, 300);
sf::RectangleShape point(sf::Vector2f(10, 10));
//struct PointData {
    //sf::Vector2f position;
    //sf::Color color;
//};
//std::vector<PointData> points;
sf::VertexArray points(sf::Points, spectW * 256);
std::vector<double> soundPiece(256);
std::vector<double> spectrum(256);

//главная функция main
int main()
{
    sf::RenderWindow window;
    window.setFramerateLimit(60);
    pg.create(500, 350); // текстура pg 500 на 350

    sf::Vector2i centerWindow((sf::VideoMode::getDesktopMode().width / 2) - 445, (sf::VideoMode::getDesktopMode().height / 2) - 480);
    window.create(sf::VideoMode(500, 500), "SFML Project", sf::Style::Titlebar | sf::Style::Close);
    window.setPosition(centerWindow);

    window.setKeyRepeatEnabled(true);



    sf::Font arial;
    arial.loadFromFile("arial.ttf");

    //создаём кнопки

    Button load_ref1("LOAD AUDIO", { 100,25 }, 15, sf::Color::Green, sf::Color::Black);
    load_ref1.setFont(arial);
    load_ref1.setPosition({ 10,10 });

    Button create_ref1("CREATE AUDIO", { 100,25 }, 14, sf::Color::Green, sf::Color::Black);
    create_ref1.setFont(arial);
    create_ref1.setPosition({ 10,40 });

    Button load_image1("LOAD IMAGE", { 100,25 }, 14, sf::Color::Green, sf::Color::Black);
    load_image1.setFont(arial);
    load_image1.setPosition({ 10,100 });

    Button encrypt1("ENCRYPT", { 100,25 }, 14, sf::Color::Green, sf::Color::Black);
    encrypt1.setFont(arial);
    encrypt1.setPosition({ 10,130 });
    
    //создаём слайдеры

    SliderSFML audio_size1(25, 375);
    SliderSFML ref_vol1(25, 425);
    SliderSFML img_vol1(25, 475);
    SliderSFML spect_w1(275, 475);

    audio_size1.setFont(arial);
    ref_vol1.setFont(arial);
    img_vol1.setFont(arial);
    spect_w1.setFont(arial);

    audio_size1.create(0,30);
    ref_vol1.create(0,200);
    img_vol1.create(0, 200);
    spect_w1.create(0, 370);

    audio_size1.setSliderValue(5);
    ref_vol1.setSliderValue(100);
    img_vol1.setSliderValue(100);
    spect_w1.setSliderValue(370);

    //устанавлием колбэки для слайдеров

    audio_size1.setCallback(audio_size);
    ref_vol1.setCallback(ref_vol);
    img_vol1.setCallback(img_vol);
    spect_w1.setCallback(spect_w);



    // список кнопок
    Button buttons[] = { load_ref1, create_ref1,load_image1,encrypt1 };


    while (window.isOpen()) {
        sf::Event Event;
        while (window.pollEvent(Event)) {
            if (Event.type == sf::Event::Closed) {
                window.close();
            }
            if (Event.type == sf::Event::MouseButtonPressed) {
                if (Event.mouseButton.button == sf::Mouse::Left) {
                    if (load_ref1.isMouseOver(window)) {
                        load_ref();
                    }
                    else if (load_image1.isMouseOver(window)) {
                        load_image();
                    }
                    else if (encrypt1.isMouseOver(window)) {
                        encrypt();
                    }
                    else if (create_ref1.isMouseOver(window)) {
                        create_ref();
                    }
                }
            }



            if (Event.type == sf::Event::MouseMoved) {
                for (auto& button : buttons) {
                    if (button.isMouseOver(window)) {
                        button.setBackColor(sf::Color::White);
                    }
                    else {
                        button.setBackColor(sf::Color::Green);
                    }
                }

            }
        }

        window.clear(sf::Color(130, 130, 130));
        pg.clear(sf::Color(130, 130, 130));
        if (updFlag) {
            updFlag = false;
          

            //визуализация аудиоданных
            if (refPath.length() != 0 || createFlag) { 
                for (int i = 0; i < volSize; i++) { // Делим данные на равные части (объемы)
                    int part = ref.size() / volSize; // Вычисляем размер каждой части
                    float maxVal = 0.0f; // Для хранения максимального значения амплитуды в текущей части
                    for (int j = 0; j < part; j++) {  // Итерируем по каждому элементу в части
                        // Ищем максимальное значение амплитуды в этой части
                        if (ref[i * part + j] > maxVal) maxVal = ref[i * part + j]; 
                    }
                    maxVal *= refAmpl; // Масштабируем максимальное значение амплитуды

                    // Настраиваем позицию первой точки линии (верх)
                    lines1[i * 2].position = sf::Vector2f(vol1X + i, 60);
                    // Настраиваем позицию второй точки линии (низ), используя maxVal для определения высоты
                    lines1[i * 2 + 1].position = sf::Vector2f(vol1X + i, 60 - maxVal * 50);

                    // Устанавливаем цвет линий в белый
                    lines1[i * 2].color = sf::Color::White;
                    lines1[i * 2 + 1].color = sf::Color::White;
                }
            }
            if (imagePath.length() != 0) { // Проверяем, задан ли путь к изображению
                img.assign(img.size(), 0.0f); // Обнуляем вектор img 
                // Проходим по всем пикселям изображения по горизонтали
                for (int i = 0; i < resizedImage.getSize().x; i++) { 
                    // Для каждой вертикальной линии пикселей до фиксированной высоты 128
                    for (int j = 0; j < 128; j++) {
                        // Получаем цвет пикселя и инвертируем j, чтобы начать с нижней части изображения
                        sf::Color pixelColor = resizedImage.getPixel(i, 127 - j);
                        // Вычисляем яркость пикселя, используя коэффициенты для RGB
                        int val = 0.299f * pixelColor.r + 0.587f * pixelColor.g + 0.114f * pixelColor.b;
                       
                        // Для каждого значения яркости генерируем волну, добавляя к каждому элементу в img
                        for (int k = 0; k < 256; k++) {
                            img[i * 256 + k] += val / 255.0f * 0.006f * std::sin((k + j * phaseVal) / framerate * 2 * M_PI * (500 + j * 150));
                           
                        }
                    }
                }

                // Для создания визуализации разделяем обработанные данные на части
                for (int i = 0; i < volSize; i++) {
                    float maxVal = 0.0f; // Максимальное значение для текущей части
                    int part = img.size() / volSize; // Размер каждой части
                    // Ищем максимальное значение в каждой части
                    for (int j = 0; j < part; j++) {
                        if (img[i * part + j] > maxVal) {
                            maxVal = img[i * part + j];
                        }
                    }
                    maxVal *= imgAmpl;  // Масштабируем максимальное значение амплитуды
                    // Устанавливаем позиции для линий визуализации на основе максимального значения
                    lines[i * 2].position = sf::Vector2f(vol2X + i, 60);
                    lines[i * 2 + 1].position = sf::Vector2f(vol2X + i, 60 - maxVal * 50);

                    // Устанавливаем цвет линий в белый
                    lines[i * 2].color = sf::Color::White;
                    lines[i * 2 + 1].color = sf::Color::White;
                }
            }
            if (refPath.length() != 0 || createFlag) { // Проверка наличия пути к файлу или установленного флага создания
                std::vector<float> fitImg(ref.size(), 0.0f); // инициализация вектора fitImg для хранения обработанных аудиоданных
                std::cout << "Ref size: " << ref.size() << std::endl;

                if (!imagePath.empty() != 0) { // Проверка наличия пути к изображению
                    // Вычисление отношения размера аудиоданных к размеру данных изображения
                    int amount = ref.size() / img.size();
                    std::cout << "REF SIZE: " << ref.size() << std::endl;
                    std::cout << "IMG SIZE: " << img.size() << std::endl;
                    std::cout << "Amount: " << amount << std::endl;
                    if (amount == 0) {
                        std::cerr << "Amount is zero, returning early." << std::endl;
                        return 1;  // возврат ошибки,если amount равен нулю
                    } 
                    // Распределение данных изображения по аудиоданным
                    int counter = 0;
                    for (size_t i = 0; i < img.size() / 256; ++i) { //? size_t i = 0;
                        for (int j = 0; j < amount; ++j) {
                            for (int k = 0; k < 256; ++k) {
                                // Заполнение fitImg данными из img, с проверкой на выход за границы
                                if (counter < fitImg.size()) {
                                    fitImg[counter++] = img[i * 256 + k];
                                }
                            }
                        }
                    }
                }
                // Подготовка к FFT преобразованию
                int parts = ref.size() / spectW; // Разделение аудиоданных на части для спектрограммы
                for (int i = 0; i < spectW; i++) {
                    std::fill(soundPiece.begin(), soundPiece.end(), 0.0); // Обнуляем буфер перед использованием
                    std::fill(spectrum.begin(), spectrum.end(), 0.0); // Обнуляем спектр

                    // Комбинирование данных ref и fitImg с учетом амплитуд
                    for (int j = 0; j < 256; j++) {
                        soundPiece[j] = ref[i * parts + j] * refAmpl + fitImg[i * parts + j] * imgAmpl;
                    }

                    // Производим FFT преобразование
                    FFTAnalysis(soundPiece.data(), spectrum.data(), 256, 256);
                    // Визуализация результатов FFT
                    for (int j = 0; j < 256; j++) {
                        double intensityRaw = spectrum[j / 2] * 30000;
                        float intensity = static_cast<float>(std::min(std::max(intensityRaw, 0.0), 255.0));

                        // Установка позиции и цвета для каждой точки спектрограммы
                        points[i * 256 + j].position = sf::Vector2f(spectX + i, spectY + 256 - j);
                        points[i * 256 + j].color = sf::Color(intensity, intensity, intensity);
                    }
                }
                // Если установлен флаг сохранения, обновляем fitImg и сохраняем аудио
                if (saveFlag) {
                    saveFlag = false;
                    for (int i = 0; i < fitImg.size(); i++) {
                        fitImg[i] = ref[i] * refAmpl + fitImg[i] * imgAmpl;
                    }

                    processSound(fitImg, 1);
                }
            }
        }
        sf::RectangleShape rect1(sf::Vector2f(150, 50));
        rect1.setPosition(150, 10);
        rect1.setOutlineColor(sf::Color::Black);
        rect1.setOutlineThickness(1);
        rect1.setFillColor(sf::Color(130, 130, 130));

        sf::RectangleShape rect2(sf::Vector2f(150, 50));
        rect2.setPosition(340, 10);
        rect2.setOutlineColor(sf::Color::Black);
        rect2.setOutlineThickness(1);
        rect2.setFillColor(sf::Color(130, 130, 130));


        sf::RectangleShape rect3(sf::Vector2f(370, 128 * 2));
        rect3.setPosition(120, 80);
        rect3.setOutlineColor(sf::Color::Black);
        rect3.setOutlineThickness(1);
        rect3.setFillColor(sf::Color(130, 130, 130));

        pg.draw(rect1);
        pg.draw(rect2);
        pg.draw(rect3);
        pg.draw(lines);
        pg.draw(points);
        pg.draw(lines1);
        pg.display();
        sf::Sprite sprite(pg.getTexture());
        sprite.setPosition(0, 0);
        window.draw(sprite);

        for (auto& button : buttons) {
            button.drawTo(window);
        }

       ref_vol1.draw(window);
        audio_size1.draw(window);
        img_vol1.draw(window);
        spect_w1.draw(window);
        window.display();
    }


    return 0;
}
void ref_vol(float val) {
    refAmpl = val / 100.0;
    updFlag = true;
}

void img_vol(float val) {
    imgAmpl = val / 100.0;
    updFlag = true;
}
void spect_w(float val) {
    spectW = val;
    updFlag = true;
}
void audio_size(float val) {
    audioSize = val;
    updFlag = true;
}
void phase(float val) {
    phaseVal = val;
    updFlag = true;
}
// кнопка шифровки
void encrypt() {
    if (refPath.length() != 0 || createFlag) {
        saveFlag = true;
        updFlag = true;
        std::cout << "WORKS" << std::endl;
    }
}

void load_ref() {
    const char* filterPatterns[2] = { "*.wav", "*.mp3" };

    const char* filePath = tinyfd_openFileDialog(
        "Select a file to process:",
        "",
        2,
        filterPatterns,
        NULL,
        0
    );
    createFlag = false;
    if (filePath) {
        selectRef(filePath);
        std::cout << "File path: " << filePath << std::endl;
    }
    else {
        std::cout << "Window was closed or the user hit cancel." << std::endl;
    }
}

void create_ref() {
    framerate = 44100;
    phaseVal = phaseShift * framerate / 44100;
    ref.resize(framerate * audioSize);
    updFlag = true;
    createFlag = true;
}

void selectRef(const std::string& selection) {
    if (!selection.empty()) {
        refPath = selection; //устанавливаем путь к файлу
        std::cout << "Ref path: " << refPath << std::endl;
        SF_INFO sfinfo; // структура для хранения информации о файле
        SNDFILE* file = sf_open(selection.c_str(), SFM_READ, &sfinfo); // открываем файл для чтения
        if (file == nullptr) {
            std::cerr << "Failed to open file: " << selection << std::endl;
            return;
        }
        std::vector<float> newRef(sfinfo.frames * sfinfo.channels); // создаем вектор для сырых аудиоданных
        sf_read_float(file, newRef.data(), sfinfo.frames * sfinfo.channels); // читаем данные из файла
        ref.resize(sfinfo.frames); // подготавливаем вектор для обработанных данных,устанавливаем размер
        for (size_t i = 0; i < ref.size(); i++) {
            // Присваивает значения из первого канала
            ref[i] = newRef[i * sfinfo.channels];

            
                /*float sum = 0;
                for (int j = 0; j < sfinfo.channels; j++) {
                    sum += newRef[i * sfinfo.channels + j];
                }
                ref[i] = sum / sfinfo.channels; // Усреднение значений по всем каналам*/

        }
        sf_close(file); // закрываем файл

        // расчёт фазового сдвига ,применяемого к выборке
        int framerate = static_cast<int>(sfinfo.samplerate);
        float phaseVal = phaseShift * framerate / 44100.0f;
        if (!ref.empty()) {
            std::cout << "1 sample: " << ref[0] << std::endl;
        }



        std::cout << "Sample Count: " << sfinfo.frames << std::endl;
        std::cout << "Channel Count: " << sfinfo.channels << std::endl;
        std::cout << "Duration: " << static_cast<double>(sfinfo.frames) / sfinfo.samplerate << " seconds" << std::endl;
        std::cout << "Framerate: " << sfinfo.samplerate << " Hz" << std::endl;

        updFlag = true; // Устанавливаем флаг обновления данных
    }
}

void load_image() {
    const char* filterPatterns[2] = { "*.png", "*.jpg" };

    const char* filePath = tinyfd_openFileDialog(
        "Select a file to process:",
        "",
        2,
        filterPatterns,
        NULL,
        0
    );

    if (filePath) {
        selectImage(filePath);
    }
    else {
        std::cout << "Window was closed or the user hit cancel." << std::endl;
    }
}

void selectImage(const std::string& selection) {
    if (selection.empty()) {
        std::cerr << "No file selected." << std::endl;
        return;
    }

    imagePath = selection; // сохраняем путь к файлу

    // проверка на загрузку изображения из файла
    if (!imageCrypt.loadFromFile(imagePath)) {
        std::cerr << "Failed to load image: " << selection << std::endl;
        return;
    }

    // Изменение размера с сохранением аспекта
    float scaleFactor = static_cast<float>(128) / imageCrypt.getSize().y;
    unsigned int newWidth = static_cast<unsigned int>(imageCrypt.getSize().x * scaleFactor);

    sf::RenderTexture renderTexture; // Создаем текстуру для изменения размера
    if (!renderTexture.create(128, 128)) {
        std::cerr << "Failed to create render texture for resizing." << std::endl; // Ошибка создания текстуры
        return;
    }
    std::cout << "Original image size: " << imageCrypt.getSize().x << " x " << imageCrypt.getSize().y << std::endl;

    sf::Texture texture; // Текстура для отрисовки
    texture.loadFromImage(imageCrypt);  // Загружаем изображение в текстуру
    texture.setSmooth(true);  // Сглаживание
    sf::Sprite sprite(texture); // Создаем спрайт
    sprite.setScale(scaleFactor, scaleFactor); // Масштабируем
    renderTexture.clear(); // Очищаем текстуру
    renderTexture.draw(sprite); // Рисуем спрайт на текстуре
    renderTexture.display(); // Обновляем текстуру
    // Retrieve the resized image
    resizedImage = renderTexture.getTexture().copyToImage(); // Получаем измененное изображение
    std::cout << "Resized image size: " << resizedImage.getSize().x << " x " << resizedImage.getSize().y << std::endl;

    // Конвертация в оттенки серого и заполнение вектора img
    for (unsigned int y = 0; y < resizedImage.getSize().y; ++y) { // Проходим по каждой строке пикселей
        for (unsigned int x = 0; x < resizedImage.getSize().x; ++x) { // Проходим по каждому столбцу пикселей
            sf::Color pixelColor = resizedImage.getPixel(x, y); // Получаем цвет текущего пикселя
            // Вычисляем значение в оттенках серого с использованием весовых коэффициентов для RGB
            uint8_t grayscaleValue = static_cast<uint8_t>(
                0.299 * pixelColor.r +
                0.587 * pixelColor.g +
                0.114 * pixelColor.b);
            // Устанавливаем новый цвет пикселя в оттенках серого
            resizedImage.setPixel(x, y, sf::Color(grayscaleValue, grayscaleValue, grayscaleValue));
        }
    }

    // Обновляем вектор img значениями яркости в оттенках серого
    img.clear(); // Очищаем вектор для удаления предыдущих данных
    for (unsigned int y = 0; y < resizedImage.getSize().y; ++y) {  // Проходим по каждой строке пикселей
        for (unsigned int x = 0; x < resizedImage.getSize().x; ++x) { // Проходим по каждому столбцу пикселей
            sf::Color pixelColor = resizedImage.getPixel(x, y); // Получаем цвет текущего пикселя в оттенках серого
            // Вычисляем яркость пикселя (значение красного канала, так как изображение в оттенках серого, все оттекни будут равны)
            float brightness = pixelColor.r / 255.0f;  // 8-бит канал , итог - значение от 0.0 до 1.0(максимальная яркость)
            // Добавляем значение яркости в вектор дважды
            img.push_back(brightness);
            img.push_back(brightness);
        }
    }
    std::cout << "Img length: " << img.size() << std::endl;
    if (resizedImage.saveToFile("grayscale_image.png")) {// Попытка сохранить изображение
        std::cout << "Grayscale image saved successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to save grayscale image." << std::endl;
    }

    // Флаг ,что изображение обновилось
    updFlag = true;

}

void processSound(const std::vector<float>& floatData, int chs) {
    // Создаем вектор для PCM данных (Pulse-code modulation) в формате short
    std::vector<short> pcm_data(floatData.size()); 

    // Преобразуем данные из float в PCM формат
    for (int i = 0; i < floatData.size(); i++) {
        // Масштабируем float значения до диапазона 16-битного звука и округляем их (floatdata[i] = от -1.0 до 1.0)
        int aux = std::floor(32767 * floatData[i ]);
        // Приводим значения к типу short и сохраняем в векторе pcm_data
        pcm_data[i] = static_cast<short>(aux);
    }

    // Инициализируем структуру информации о звуковом файле
    SF_INFO sfinfo;
    sfinfo.channels = chs; // Количество каналов (1 для моно, 2 для стерео и т.д.)
    sfinfo.samplerate = framerate; // Частота дискретизации(количество семплов в секунду)
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; // Указываем формат файла (WAV) и кодек (PCM 16 бит)

    // Открываем файл для записи с указанными параметрами
    SNDFILE* outfile = sf_open("new.wav", SFM_WRITE, &sfinfo);
    if (outfile == nullptr) {
        std::cerr << "Error opening output file.\n";
        return;
    }
    // Записываем PCM данные в файл
    sf_write_short(outfile, pcm_data.data(), pcm_data.size());
    // Закрываем файл
    sf_close(outfile);
}
