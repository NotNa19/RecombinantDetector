# En
## About 
The repository contains a recombinant detection program based on the 3SEQ algorithm.
>Lam HM, Ratmann O, Boni MF. Improved algorithmic complexity for the 3SEQ recombination detection algorithm. Mol Biol Evol, 35(1):247-251, 2018.
>
>Boni MF, Posada D, Feldman MW. An exact nonparametric method for inferring mosaic structure in sequence triplets. Genetics, 176:1035-1047, 2007.


As in the original implementation, user needs aligned sequences in fasta format in the input, which are tested for the presence of recombinant triples using a statistical test (described in the articles above).
The table of P-values must be calculated in advance — the program also provides this functionality.

Compared to the original authors implementation, we support multithreading here, as well as various large and small optimizations.
One of the major optimizations is the ability to use header information from the fasta file (you can see the details below). Enabling this functionality for some tests allowed us to achieve a twofold increase in speed.
In theory, for some cases this can speed up the work dozens of times.

## Quick start
To build a project on Linux using CMake, you need to follow these steps:
1. Download and unzip the project files.
2. Create a `build` folder in the root folder of the project (near `CMakeLists.txt` file) and navigate to it.
3. Execute `cmake -DCMAKE_BUILD_TYPE=Release ..`.
4. Execute `make`.

Before starting, you need to generate a P-values table. You can do this for the first time and then reuse it.
1. In the `build` folder execute `./RecDetector -gen-p table500 500`. The generation may take several minutes.
2. Move the `table500` to a convenient place.
3. Open the `source/settings/UserSettings.rec`.
4. Find the `pTableFilePath` entry and specify the path to the table.

Now you can finally run the application.
1. Download any `.fasta` file with aligned sequences (we will use `testSeqs/ebola_aligned.fasta` for example).
2. Execute `./RecDetector -detect testSeqs/ebola_aligned.fasta`. Wait for the end of the process.
3. Now you can find several files with the results in the `build` folder. The most interesting one is `results.csv'. It provides detailed information about the recombinants found.

# Ru
## О программе
Репозиторий содержит программу по поиску рекомбинантов, основанную на алгоритме 3SEQ. 
>Lam HM, Ratmann O, Boni MF. Improved algorithmic complexity for the 3SEQ recombination detection algorithm. Mol Biol Evol, 35(1):247-251, 2018.
>
>Boni MF, Posada D, Feldman MW. An exact nonparametric method for inferring mosaic structure in sequence triplets. Genetics, 176:1035-1047, 2007.


Как и в оригинальной реализации, на вход подаются выровненные последовательности в fasta формате, которые проверяются на наличие рекомбинантных троек. Для проверки используется статистический тест (описанный в работах выше).
Таблица P-значений должны быть посчитана заранее — программа также предоставляет эту функциональность.

По сравнению с оригинальной реализацией авторов, здесь добавлена поддержка многопоточности, а также различные крупные и мелкие оптимизации.
Одной из серьезных оптимизаций является возможность использования заголовочной информации из fasta файла (инструкция будет ниже). Включение этой функциональности для некоторых тестов позволила нам добиться двукратного прироста скорости.
При этом, в теории, для некоторых случаев это может позволить ускорить работу в десятки раз.

## Быстрый старт
Для сборки проекта на Linux c использованием CMake выполните следующие шаги:
1. Скачайте и распакуйте файлы проекта.
2. Создайте папку `build` в корневой папке проекта (рядом с `CMakeLists.txt`) и перейдите в неё.
3. В терминале выполните `cmake -DCMAKE_BUILD_TYPE=Release ..`.
4. В терминале выполните `make`.

Перед запуском необходимо сгенерировать таблицу P-значений. Можно сделать это для первого раза и затем переиспользовать её.
1. В папке `build` выполните `./RecDetector -gen-p table500 500`. Генерация может занять несколько минут.
2. Переместите получившуюся таблицу в удобное место.
3. Откройте `source/settings/UserSettings.rec`.
4. Найдите поле `pTableFilePath` и укажите путь к таблице.

Теперь можно провести тестовый запуск приложения.
1. Скачайте любой `.fasta` файл с выровненными последовательностями (далее для примера будем использовать `testSeqs/ebola_aligned.fasta`).
2. Выполните `./RecDetector -detect testSeqs/ebola_aligned.fasta`.
3. После окончания работы в папке build будет создано несколько файлов с результатами. Самый интересный — `results.csv`. В нём можнно получить подробную информацию о найденных рекомбинантах.

**Пример таблицы с результатами:**

![image](https://github.com/user-attachments/assets/c24e6d34-691f-46e9-b1d0-a3394401b5d9)

## Советы по работе с приложением
1. Входные данные приложения — fasta файлы, содержащие уже **выровненные** геномы.  
2. Выходными данными являются несколько файлов. Для первичного анализа результатов будут интересны `RecDetector.log` и `results.csv` (пример выше).
3. На вход можно подавать несколько файлов с последовательностями. В этом случае команда для запуска будет, например `./RecDetector -detect -dir testSeqs`. Где `testSeqs` — директория, содержащая несколько fasta файлов.
4. Лучше заранее сгенерировать большую таблицу для P-значений. Для нашего тестирования мы использовали таблицу размером 1000x1000x1000 (`./RecDetector -gen-p table1000 1000`). Она занимает ~2гб на диске, при этом её размера хватает для большинства входных данных.
   


## `UserSettings.rec` файл и настройка приложения
Пользователь может использовать `source/setings/UserSettings.rec` файл для настройки приложения. Ниже приведены самые важные. Стоит отметить, что в текущей реализации порядок настроек в файле важен — пользователю рекомендуется менять только значения. 

#### 1. Работа с заголовками fasta файлов
Зачастую fasta файлы имеют достаточно важную информацию в своём заголовке. Например, это может выглядеть так:
>CY066190 A/Boston/144/2009 2009/07/09 1 (PB2).

Можно заметить, что строчка содержит в себе дату секвенирования (2009/07/09). Эта информация может помочь нам сократить число троек для анализа. 
Очевидно, что в тройке последовательностей (`Parent1`, `Parent2`, `Child`), геном `Child` не может быть результатом рекомбинации `Parent1` и `Parent2`, если секвенирование генома `Child` произошло раньше чем секвенирование `Parent1` или `Parent2`.

Для включения проверок такого типа мы используем флаг `useHeaderData`. Флаг `timeThresholdBetweenParentsAndChild` необходим для того, чтобы указать допустимый промежуток секвенирования родителя и ребенка (в секундах).
Например, при таких настройках в `UserSettings.rec`:

>useHeaderData 1  
>timeThresholdBetweenParentsAndChild 2678400

Мы будем исключать тройки, где `Parent1` или `Parent2` младше `Child` на месяц и больше.

#### 2. Работа с потоками
Программа поддерживает мультипоточность. Поле `threadsCount` используется для управления числом используемых потоков.

#### 3. Путь к таблице P-значений
Пользователь может изменить путь к таблице P-значений. Пример:
>pTableFilePath /home/adev/Work/ptables/table1000

### 4. Путь к выходным данных
Пользователь может изменить путь к директории, где будут хранится файлы с результатами. Пример:
>outputDirPath /home/adev/Work/results/run_1234

Для использования директории по умолчанию следует использовать (будет исправлено):
>outputDirPath _

#### 5. Другие параметры
В файле `UserSettings.rec` также хранятся некоторые настройки, влияющие на работу алгоритма. Смысл большинства понятен из названия, но в будущем руководство будет дополнено и этими деталями.

## Тестовые данные
Для тестирования приложения мы использовали искуственно созданные и реальные данные.
Скрипты для генерации искуственных последовательностей можно найти в `genomeData/generation`. Там же доступны некоторые примеры сгенерированных fasta файлов. Позже документация будет дополнена инструкцией по использованию этих скриптов.  

В папке `genomeData/viruses` можно найти 4 набора геномов. Это грипп A H1N1 (белок PB2), лихорадка Денге (серотип 2, Азия) , вирус Эбола (заирский штамм) и коронавирус SARS-CoV-2 (Россия).
