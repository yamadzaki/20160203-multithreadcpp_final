# 20160203-multithreadcpp_final
Final task in multithread cpp course.

What to do:

Ну вот мы и подошли к последней задаче!

Работа большая, оценивается в 300 баллов.

1. Нужно создать проект на github и написать веб-сервер. Протокол HTTP 1.0, нужно реализовать только команду GET (POST - опционально), ответы 200 и 404, а также MIME-тип text/html (другие типы, например image/jpeg - опционально).

2. Запустить виртуальную машину и сохранить http-путь до вашего github репозитория в файле /home/box/final.txt

3. Первым делом тестовая среда проверяет наличие этого файла и сама клонирует репозиторий в /home/box/final.

4. Проект должен собираться с помощью cmake (изучается самостоятельно). На виртуалке стоит cmake версии 2.8. Тестовая среда в папке /home/box/final выполняет команды:

cmake .

make

5. В результате сборки должен появиться исполняемый файл /home/box/final/final - ваш веб-сервер. Тестовая среда проверяет его наличие.

6. Веб-сервер должен запускаться командой:

/home/box/final/final -h <ip> -p <port> -d <directory>

Для парсинга параметров командной строки используйте getopt (изучается самостоятельно). После запуска сервер обязан вернуть управление, то есть должен демонизироваться. Иначе тесты встанут и отвалятся по таймауту!

7. Тестовая среда исполняет HTTP-запросы.

8. Самое главное. Сервер должен быть или многопоточным или многопроцессным с передачей дескрипторов.

9. Следующие трюки считаются читерством:

    Держать в памяти настроенный nginx (или, например, apache) при наличии программы-заглушки в github.
    Использовать реализацию HTTP-протокола из libevent.


Успехов!
