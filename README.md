Java Class Dumper - Инструкция по использованию
Описание
Java Class Dumper - это агент JVMTI, написанный на C++, который позволяет извлекать (дампить) байт-код всех классов, загружаемых в JVM во время выполнения программы.
Функциональность

Автоматический перехват: Агент перехватывает все события загрузки классов
Сохранение в файлы: Сохраняет .class файлы с правильной структурой директорий
Поддержка всех типов классов: Обычные классы, внутренние классы, анонимные классы, lambda-выражения
Кросс-платформенность: Linux, macOS, Windows

Компиляция
Требования

JDK 8 или выше
GCC или Clang
Make

Сборка
bash# Убедитесь что JAVA_HOME установлена правильно
export JAVA_HOME=/path/to/your/jdk

# Соберите агент и тестовое приложение
make all

# Или только агент
make classdumper
Для Windows с MinGW:
cmdset JAVA_HOME=C:\Program Files\Java\jdk-11
make all
Использование
Базовое использование
bash# Запуск Java приложения с агентом
java -javaagent:./classdumper.so=./output_directory YourMainClass

# Для Windows
java -javaagent:./classdumper.dll=./output_directory YourMainClass

# Для macOS  
java -javaagent:./classdumper.dylib=./output_directory YourMainClass
Тестирование
bash# Запустить тестовое приложение
make run-test

# С подробным выводом загрузки классов
make run-test-verbose
Параметры агента

Параметр агента указывает директорию для сохранения .class файлов
Если не указан, используется ./dumped_classes/

Структура вывода
Агент сохраняет классы в следующей структуре:
output_directory/
├── java/
│   ├── lang/
│   │   ├── String.class
│   │   ├── Object.class
│   │   └── ...
│   └── util/
│       ├── ArrayList.class
│       ├── HashMap.class
│       └── ...
├── your/package/
│   ├── YourMainClass.class
│   ├── YourMainClass$InnerClass.class
│   └── ...
└── ...
Примеры использования
1. Дамп классов Spring Boot приложения
bashjava -javaagent:./classdumper.so=./spring_classes -jar myapp.jar
2. Анализ загрузки классов в Maven тесте
bashmvn test -DargLine="-javaagent:/path/to/classdumper.so=./test_classes"
3. Исследование классов в JAR файле
bashjava -javaagent:./classdumper.so=./jar_classes -cp mylib.jar com.example.Main
Отладка и логирование
Агент выводит информацию о своей работе:
[DUMPER] Agent loading...
[DUMPER] Output directory: ./output/
[DUMPER] Agent loaded successfully
[DUMPER] Saved: ./output/TestApp.class (1234 bytes)
[DUMPER] Saved: ./output/TestApp$InnerTestClass.class (567 bytes)
[DUMPER] Class prepared: Ljava/lang/String;
[DUMPER] Found 150 loaded classes
[DUMPER] Agent unloading...
[DUMPER] Agent unloaded
Ограничения

Системные классы: Некоторые системные классы могут быть недоступны для дампа
Права доступа: Агент может не работать с некоторыми SecurityManager настройками
Производительность: Дамп большого количества классов может замедлить запуск приложения

Устранение неполадок
Ошибки компиляции
"класс jvmtiCapabilities не содержит члена can_get_loaded_classes"

Исправлено: Удалено неправильное поле can_get_loaded_classes
GetLoadedClasses() доступна без специальных capabilities

"идентификатор mkdir не определен"

Исправлено: Добавлены правильные заголовки для каждой платформы
Windows: #include <direct.h> и _mkdir
Unix: #include <sys/stat.h> и mkdir

Ошибки выполнения
"Failed to get JVMTI environment"

Убедитесь, что используете полную JDK, а не JRE
Проверьте версию Java (требуется 8+)
Для Windows: убедитесь что используете правильную разрядность (x86/x64)

Ошибка загрузки библиотеки
bash# Linux: проверьте зависимости
ldd classdumper.so

# macOS: проверьте архитектуру
file classdumper.dylib
otool -L classdumper.dylib

# Windows: проверьте зависимости
dumpbin /dependents classdumper.dll
Агент не сохраняет файлы

Проверьте права доступа к директории вывода
Убедитесь, что путь корректен и существует
Проверьте логи агента на наличие ошибок
На Windows используйте полные пути или экранируйте обратные слеши

Проблемы с компилятором
MinGW на Windows
cmd# Установка через Chocolatey
choco install mingw

# Или загрузите с https://www.mingw-w64.org/
Visual Studio
cmd# Используйте Developer Command Prompt
# Замените g++ на cl.exe в команде компиляции
Отладочная информация
bash# Включить подробный вывод JVMTI
java -XX:+TraceClassLoading -javaagent:classdumper.so=./output MyApp

# Проверить версию JVMTI
java -agentlib:jdwp=help 2>&1 | head -5
Продвинутое использование
Настройка через переменные окружения
bashexport CLASSDUMPER_OUTPUT=/tmp/classes
export CLASSDUMPER_FILTER="com.mycompany.*"
java -javaagent:./classdumper.so MyApp
Интеграция с IDE
Добавьте в VM options вашей IDE:
-javaagent:/path/to/classdumper.so=/path/to/output
Анализ результатов
bash# Подсчет классов
find output_directory -name "*.class" | wc -l

# Поиск определенных классов
find output_directory -name "*Lambda*" -type f

# Анализ размеров
du -sh output_directory/*
Безопасность

Агент работает с полными правами JVM
Сохраненные классы могут содержать конфиденциальную информацию
Используйте только в доверенных окружениях
Не используйте в production без тщательного тестирования
