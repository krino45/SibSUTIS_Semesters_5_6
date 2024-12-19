#include "searchpage.h"

SearchPage::SearchPage(QString text_param, QString category_param, QWidget *parent) : QWidget(parent) {
    initialize();
    searchLineEdit->setText(text_param);
    categoryDropdown->setCurrentText(category_param);
    performSearch(categoryDropdown->currentText());
}

SearchPage::SearchPage(QWidget *parent) : QWidget(parent) {
    initialize();
    performSearch();

}

void SearchPage::initialize() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);



    // Заголовок
    QLabel *titleLabel = new QLabel("Поиск", this);
    titleLabel->setStyleSheet("font-size: 46px; font-weight: 400; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Поля ввода поиска в одной линии
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(0);

    // Иконка поиска
    QLabel *searchIcon = new QLabel(this);
    searchIcon->setPixmap(QPixmap(":/images/search_icon.png").scaled(60, 60, Qt::KeepAspectRatio));
    searchIcon->setStyleSheet("padding: 10px; border: 1px solid #BCBCBC; background-color: white;");
    searchLayout->addWidget(searchIcon);

    // Поле ввода
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Искать...");
    searchLineEdit->setFixedHeight(80);
    searchLineEdit->setStyleSheet("font-size: 32px; padding: 6px; border: 1px solid #BCBCBC; border-style: solid none solid none;");
    searchLayout->addWidget(searchLineEdit, 5); // Поле поиска занимает до середины

    // Кнопка сброса (внутри поля ввода)
    QPushButton *clearButton = new QPushButton("X", searchLineEdit);
    clearButton->setStyleSheet("border: 1px solid #BCBCBC; border-style: solid none solid none; font-size: 32px; font-weight: 300; color: #888; background-color: white;");
    clearButton->setFixedSize(80, 80);
    clearButton->setCursor(Qt::PointingHandCursor);
    searchLayout->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, searchLineEdit, &QLineEdit::clear);

    // Иконка геолокации
    QLabel *geoIcon = new QLabel(this);
    geoIcon->setPixmap(QPixmap(":/images/location_icon.png").scaled(60, 60, Qt::KeepAspectRatio));
    geoIcon->setStyleSheet("padding: 10px; border: 1px solid #BCBCBC; background-color: white;");
    searchLayout->addWidget(geoIcon);

    // Поле ввода города
    locationLineEdit = new QLineEdit(this);
    locationLineEdit->setPlaceholderText("Введите локацию...");
    locationLineEdit->setFixedHeight(80);
    locationLineEdit->setStyleSheet("font-size: 32px; padding: 6px; border: 1px solid #BCBCBC; border-style: solid solid solid none");
    searchLayout->addWidget(locationLineEdit, 4); // 2/3 от оставшейся половины

    QStringList predefinedCities = {"Абаза", "Абакан", "Абдулино", "Абзаково", "Абинск", "Абрау-Дюрсо", "Авдеевка", "Агидель", "Агрыз", "Адлер", "Адыгейск", "Азнакаево", "Азов", "Ак-Довурак", "Аксай", "Алагир", "Алапаевск", "Алатырь", "Алдан", "Алейск", "Александров", "Александровск", "Александровск-Сахалинский", "Алексеевка", "Алексин", "Алзамай", "Алупка", "Алушта", "Алчевск", "Альметьевск", "Алёшки", "Цюрупинск", "Амвросиевка", "Амурск", "Анадырь", "Анапа", "Ангарск", "Андреаполь", "Анжеро-Судженск", "Анива", "Антрацит", "Апатиты", "Апрелевка", "Апшеронск", "Арамиль", "Аргун", "Ардатов", "Ардон", "Арзамас", "Аркадак", "Армавир", "Армянск", "Арсеньев", "Арск", "Артём", "Артёмово", "Артёмовск", "Артёмовский", "Архангельск", "Архипо-Осиповка", "Архыз", "Асбест", "Асино", "Астрахань", "Аткарск", "Ахтубинск", "Ачинск", "Аша", "Бабаево", "Бабушкин", "Бавлы", "Багратионовск", "Байкальск", "Баймак", "Бакал", "Баксан", "Балабаново", "Балаково", "Балахна", "Балашиха", "Балашов", "Балей", "Балтийск", "Барабинск", "Барнаул", "Барыш", "Батайск", "Бахчисарай", "Бежецк", "Белая Калитва", "Белая Холуница", "Белгород", "Белебей", "Белинский", "Белицкое", "Белово", "Белогорск", "Белозерск", "Белозёрское", "Белокуриха", "Беломорск", "Белоозёрский", "Белорецк", "Белореченск", "Белоусово", "Белоярский", "Белый", "Белёв", "Бердск", "Бердянск", "Береговое", "Березники", "Берислав", "Берёзовский", "Беслан", "Бийск", "Бикин", "Билибино", "Биробиджан", "Бирск", "Бирюсинск", "Бирюч", "Благовещенск", "Благовещенская", "Благодарный", "Бобров", "Богданович", "Богородицк", "Богородск", "Боготол", "Богучар", "Бодайбо", "Бокситогорск", "Болгар", "Бологое", "Болотное", "Болохово", "Болхов", "Большой Камень", "Большой Утриш", "Бор", "Борзя", "Борисоглебск", "Боровичи", "Боровск", "Бородино", "Братск", "Бронницы", "Брянка", "Брянск", "Бугульма", "Бугуруслан", "Будённовск", "Бузулук", "Буинск", "Буй", "Буйнакск", "Бутурлиновка", "Валдай", "Валуйки", "Васильевка", "Вахрушево", "Велиж", "Великие Луки", "Великий Новгород", "Великий Устюг", "Вельск", "Венёв", "Верещагино", "Верея", "Верхнеуральск", "Верхний Тагил", "Верхний Уфалей", "Верхняя Пышма", "Верхняя Салда", "Верхняя Тура", "Верхотурье", "Верхоянск", "Веселовка", "Весьегонск", "Ветлуга", "Видное", "Вилюйск", "Вилючинск", "Витязево", "Вихоревка", "Вичуга", "Владивосток", "Владикавказ", "Владимир", "Волгоград", "Волгодонск", "Волгореченск", "Волжск", "Волжский", "Волноваха", "Вологда", "Володарск", "Волоколамск", "Волосово", "Волхов", "Волчанск", "Вольнянск", "Вольск", "Воркута", "Воронеж", "Ворсма", "Воскресенск", "Воткинск", "Всеволожск", "Вуктыл", "Выборг", "Выкса", "Высоковск", "Высоцк", "Вытегра", "Вышний Волочёк", "Вяземский", "Вязники", "Вязьма", "Вятские Поляны", "Гаврилов Посад", "Гаврилов-Ям", "Гагарин", "Гаджиево", "Гай", "Галич", "Гаспра", "Гатчина", "Гвардейск", "Гдов", "Геленджик", "Геническ", "Георгиевск", "Глазов", "Голая Пристань", "Голицыно", "Голубицкая", "Горбатов", "Горловка", "Горно-Алтайск", "Горнозаводск", "Горняк", "Городец", "Городище", "Городовиковск", "Гороховец", "Горское", "Горячий Ключ", "Грайворон", "Гремячинск", "Грозный", "Грязи", "Грязовец", "Губаха", "Губкин", "Губкинский", "Гудермес", "Гуково", "Гулькевичи", "Гуляйполе", "Гурзуф", "Гурьевск", "Гусев", "Гусиноозёрск", "Гусь-Хрустальный", "Давлеканово", "Дагестанские Огни", "Дагомыс", "Далматово", "Дальнегорск", "Дальнереченск", "Данилов", "Данков", "Дебальцево", "Дегтярск", "Дедовск", "Демидов", "Дербент", "Десногорск", "Джанкой", "Джемете", "Джубга", "Дзержинск", "Дзержинский", "Дивеево", "Дивногорск", "Дивноморское", "Дигора", "Димитров", "Димитровград", "Дмитриев", "Дмитров", "Дмитровск", "Днепрорудное", "Дно", "Доброполье", "Добрянка", "Докучаевск", "Долгопрудный", "Должанская", "Долинск", "Домбай", "Домодедово", "Донецк", "Донской", "Дорогобуж", "Дрезна", "Дружковка", "Дубна", "Дубовка", "Дудинка", "Духовщина", "Дюртюли", "Дятьково", "Евпатория", "Егорьевск", "Ейск", "Екатеринбург", "Елабуга", "Елец", "Елизово", "Ельня", "Еманжелинск", "Емва", "Енакиево", "Енисейск", "Ермолино", "Ершов", "Ессентуки", "Ефремов", "Ждановка", "Железноводск", "Железногорск", "Железногорск-Илимский", "Жердевка", "Жигулёвск", "Жиздра", "Жирновск", "Жуков", "Жуковка", "Жуковский", "Завитинск", "Заводоуковск", "Заволжск", "Заволжье", "Задонск", "Заинск", "Закаменск", "Заозёрный", "Заозёрск", "Западная Двина", "Заполярный", "Запорожье", "Зарайск", "Заречный", "Заринск", "Звенигово", "Звенигород", "Зверево", "Зеленогорск", "Зеленоградск", "Зеленодольск", "Зеленокумск", "Зерноград", "Зея", "Зима", "Зимогорье", "Златоуст", "Злынка", "Змеиногорск", "Знаменск", "Золотое", "Зоринск", "Зубцов", "Зугрэс", "Зуевка", "Ивангород", "Иваново", "Ивантеевка", "Ивдель", "Игарка", "Ижевск", "Избербаш", "Изобильный", "Иланский", "Иловайск", "Инза", "Иннополис", "Инсар", "Инта", "Ипатово", "Ирбит", "Иркутск", "Ирмино", "Исилькуль", "Искитим", "Истра", "Ишим", "Ишимбай", "Йошкар-Ола", "Кабардинка", "Кадников", "Казань", "Калач", "Калач-на-Дону", "Калачинск", "Калининград", "Калининск", "Калтан", "Калуга", "Калязин", "Камбарка", "Каменка", "Каменка-Днепровская", "Каменногорск", "Каменск-Уральский", "Каменск-Шахтинский", "Камень-на-Оби", "Камешково", "Камызяк", "Камышин", "Камышлов", "Канаш", "Кандалакша", "Канск", "Карабаново", "Карабаш", "Карабулак", "Карасук", "Карачаевск", "Карачев", "Каргат", "Каргополь", "Карпинск", "Карталы", "Касимов", "Касли", "Каспийск", "Катав-Ивановск", "Катайск", "Каховка", "Качканар", "Кашин", "Кашира", "Каякент", "Кедровый", "Кемерово", "Кемь", "Керчь", "Кизел", "Кизилюрт", "Кизляр", "Кимовск", "Кимры", "Кингисепп", "Кинель", "Кинешма", "Киреевск", "Киренск", "Киржач", "Кириллов", "Кириши", "Киров", "Кировград", "Кирово-Чепецк", "Кировск", "Кировское", "Кирс", "Кирсанов", "Киселёвск", "Кисловодск", "Клин", "Клинцы", "Княгинино", "Ковдор", "Ковров", "Ковылкино", "Когалым", "Кодинск", "Козельск", "Козловка", "Козьмодемьянск", "Коктебель", "Кола", "Кологрив", "Коломна", "Колпашево", "Кольчугино", "Коммунар", "Комсомольск", "Комсомольск-на-Амуре", "Комсомольское", "Конаково", "Кондопога", "Кондрово", "Константиновка", "Константиновск", "Копейск", "Кораблино", "Кореновск", "Коркино", "Королёв", "Короча", "Корсаков", "Коряжма", "Костерёво", "Костомукша", "Кострома", "Котельники", "Котельниково", "Котельнич", "Котлас", "Котово", "Котовск", "Кохма", "Краматорск", "Красавино", "Красная Поляна", "Красноармейск", "Красновишерск", "Красногоровка", "Красногорск", "Краснодар", "Краснодон", "Краснозаводск", "Краснознаменск", "Краснокаменск", "Краснокамск", "Красноперекопск", "Краснослободск", "Краснотурьинск", "Красноуральск", "Красноуфимск", "Красноярск", "Красный Кут", "Красный Лиман", "Красный Луч", "Красный Сулин", "Красный Холм", "Кременная", "Кремёнки", "Кронштадт", "Кропоткин", "Крымск", "Кстово", "Кубинка", "Кувандык", "Кувшиново", "Кудрово", "Кудымкар", "Кузнецк", "Куйбышев", "Кукмор", "Кулебаки", "Кумертау", "Кунгур", "Купино", "Курахово", "Курган", "Курганинск", "Курильск", "Курлово", "Куровское", "Курск", "Куртамыш", "Курчалой", "Курчатов", "Куса", "Кучугуры", "Кушва", "Кызыл", "Кыштым", "Кяхта", "Лабинск", "Лабытнанги", "Лагань", "Ладушкин", "Лазаревское", "Лаишево", "Лакинск", "Лангепас", "Лахденпохья", "Лебедянь", "Лениногорск", "Ленинск", "Ленинск-Кузнецкий", "Ленск", "Лермонтов", "Лермонтово", "Лесной", "Лесозаводск", "Лесосибирск", "Ливны", "Ликино-Дулёво", "Липецк", "Липки", "Лисичанск", "Лиски", "Лихославль", "Лобня", "Лодейное Поле", "Лоо", "Лосино-Петровский", "Луга", "Луганск", "Луза", "Лукоянов", "Лутугино", "Луховицы", "Лысково", "Лысьва", "Лыткарино", "Льгов", "Любань", "Люберцы", "Любим", "Людиново", "Лянтор", "Магадан", "Магас", "Магнитогорск", "Майкоп", "Майский", "Макаров", "Макарьев", "Макеевка", "Макушино", "Малая Вишера", "Малгобек", "Малмыж", "Малоархангельск", "Малоярославец", "Мамадыш", "Мамоново", "Манжерок", "Мантурово", "Мариинск", "Мариинский Посад", "Мариуполь", "Маркс", "Марьинка", "Махачкала", "Мацеста", "Мглин", "Мегион", "Медвежьегорск", "Медногорск", "Медынь", "Межводное", "Межгорье", "Междуреченск", "Мезень", "Мезмай", "Меленки", "Мелеуз", "Мелитополь", "Менделеевск", "Мензелинск", "Мещовск", "Миасс", "Микунь", "Миллерово", "Минеральные Воды", "Минусинск", "Миньяр", "Мирный", "Мисхор", "Миусинск", "Михайлов", "Михайловка", "Михайловск", "Мичуринск", "Могоча", "Можайск", "Можга", "Моздок", "Молодогвардейск", "Молочанск", "Мончегорск", "Морозовск", "Морское", "Моршанск", "Мосальск", "Москва", "Моспино", "Муравленко", "Мураши", "Мурино", "Мурманск", "Муром", "Мценск", "Мыски", "Мысовое", "Мытищи", "Мышкин", "Набережные Челны", "Навашино", "Наволоки", "Надым", "Назарово", "Назрань", "Называевск", "Нальчик", "Нариманов", "Наро-Фоминск", "Нарткала", "Нарьян-Мар", "Находка", "Невель", "Невельск", "Невинномысск", "Невьянск", "Нелидово", "Неман", "Нерехта", "Нерчинск", "Нерюнгри", "Нестеров", "Нефтегорск", "Нефтекамск", "Нефтекумск", "Нефтеюганск", "Нея", "Нижневартовск", "Нижнекамск", "Нижнеудинск", "Нижние Серги", "Нижний Ломов", "Нижний Новгород", "Нижний Тагил", "Нижняя Салда", "Нижняя Тура", "Николаевка", "Николаевск", "Николаевск-на-Амуре", "Никольск", "Никольское", "Новая Анапа", "Новая Евпатория", "Новая Каховка", "Новая Ладога", "Новая Ляля", "Новоазовск", "Новоалександровск", "Новоалтайск", "Новоаннинский", "Нововоронеж", "Новогродовка", "Новодвинск", "Новодружеск", "Новозыбков", "Новокубанск", "Новокузнецк", "Новокуйбышевск", "Новомихайловский", "Новомичуринск", "Новомосковск", "Новопавловск", "Новоржев", "Новороссийск", "Новосибирск", "Новосиль", "Новосокольники", "Новотроицк", "Новоузенск", "Новоульяновск", "Новоуральск", "Новохопёрск", "Новочебоксарск", "Новочеркасск", "Новошахтинск", "Новый Оскол", "Новый Свет", "Новый Уренгой", "Ногинск", "Нолинск", "Норильск", "Ноябрьск", "Нурлат", "Нытва", "Нюрба", "Нягань", "Нязепетровск", "Няндома", "Облучье", "Обнинск", "Обоянь", "Обь", "Одинцово", "Озёрск", "Озёры", "Октябрьск", "Октябрьский", "Окуловка", "Оленевка", "Оленегорск", "Олонец", "Ольгинка", "Олёкминск", "Омск", "Омутнинск", "Онега", "Опочка", "Орджоникидзе", "Оренбург", "Орехов", "Орехово-Зуево", "Орлов", "Орск", "Орёл", "Оса", "Осинники", "Осташков", "Остров", "Островной", "Острогожск", "Отрадное", "Отрадный", "Оха", "Оханск", "Очёр", "Павлово", "Павловск", "Павловский Посад", "Палех", "Палласовка", "Партенит", "Партизанск", "Певек", "Пенза", "Первомайск", "Первоуральск", "Перевальск", "Перевоз", "Пересвет", "Переславль-Залесский", "Пересыпь", "Пермь", "Пестово", "Петергоф", "Петров Вал", "Петровск", "Петровск-Забайкальский", "Петровское", "Петрозаводск", "Петропавловск-Камчатский", "Петухово", "Петушки", "Печора", "Печоры", "Пикалёво", "Пионерский", "Питкяранта", "Плавск", "Пласт", "Плёс", "Поворино", "Подольск", "Подпорожье", "Покачи", "Покров", "Покровск", "Полевской", "Полесск", "Пологи", "Полысаево", "Полярные Зори", "Полярный", "Попасная", "Поповка", "Поронайск", "Порхов", "Похвистнево", "Почеп", "Починок", "Пошехонье", "Правдинск", "Приволжск", "Приволье", "Приморск", "Приморский", "Приморско-Ахтарск", "Приозерск", "Прокопьевск", "Пролетарск", "Протвино", "Прохладный", "Псков", "Пугачёв", "Пудож", "Пустошка", "Пучеж", "Пушкино", "Пущино", "Пыталово", "Пыть-Ях", "Пятигорск", "Радужный", "Райчихинск", "Раменское", "Рассказово", "Ревда", "Реж", "Реутов", "Ржев", "Ровеньки", "Родинское", "Родники", "Рославль", "Россошь", "Ростов", "Ростов Великий", "Ростов-на-Дону", "Рошаль", "Ртищево", "Рубежное", "Рубцовск", "Рудня", "Руза", "Рузаевка", "Рыбачье", "Рыбинск", "Рыбное", "Рыльск", "Ряжск", "Рязань", "Саки", "Салават", "Салаир", "Салехард", "Сальск", "Самара", "Санкт-Петербург", "Саранск", "Сарапул", "Саратов", "Саров", "Сасово", "Сатка", "Сафоново", "Саяногорск", "Саянск", "Сватово", "Свердловск", "Светлогорск", "Светлоград", "Светлодарск", "Светлый", "Светогорск", "Свирск", "Свияжск", "Свободный", "Святогорск", "Себеж", "Севастополь", "Северо-Курильск", "Северобайкальск", "Северодвинск", "Северодонецк", "Североморск", "Североуральск", "Северск", "Севск", "Сегежа", "Селидово", "Сельцо", "Семикаракорск", "Семилуки", "Семёнов", "Сенгилей", "Серафимович", "Сергач", "Сергиев Посад", "Сердобск", "Серов", "Серпухов", "Сертолово", "Сибай", "Сим", "Симеиз", "Симферополь", "Скадовск", "Сковородино", "Скопин", "Славгород", "Славск", "Славянск", "Славянск-на-Кубани", "Сланцы", "Слободской", "Слюдянка", "Смоленск", "Снежинск", "Снежногорск", "Снежное", "Снигирёвка", "Собинка", "Советск", "Советская Гавань", "Советский", "Сокол", "Соледар", "Солигалич", "Соликамск", "Солнечногорск", "Солнечногорское", "Соль-Илецк", "Сольвычегодск", "Сольцы", "Сорочинск", "Сорск", "Сортавала", "Сосенский", "Сосновка", "Сосновоборск", "Сосновый Бор", "Сосногорск", "Сочи", "Спас-Деменск", "Спас-Клепики", "Спасск", "Спасск-Дальний", "Спасск-Рязанский", "Среднеколымск", "Среднеуральск", "Сретенск", "Ставрополь", "Старая Купавна", "Старая Ладога", "Старая Русса", "Старица", "Старобельск", "Стародуб", "Старый Крым", "Старый Оскол", "Стаханов", "Стерлитамак", "Стрежевой", "Строитель", "Струнино", "Ступино", "Суворов", "Судак", "Суджа", "Судогда", "Суздаль", "Сукко", "Сунжа", "Суоярви", "Сураж", "Сургут", "Суровикино", "Сурск", "Сусуман", "Сухиничи", "Суходольск", "Сухой Лог", "Счастье", "Сызрань", "Сыктывкар", "Сысерть", "Сычёвка", "Сясьстрой", "Тавда", "Таврийск", "Таганрог", "Тайга", "Тайшет", "Талдом", "Талица", "Тамань", "Тамбов", "Тара", "Тарко-Сале", "Таруса", "Татарск", "Таштагол", "Тверь", "Теберда", "Тейково", "Темников", "Темрюк", "Терек", "Тетюши", "Тимашёвск", "Тихвин", "Тихорецк", "Тобольск", "Тогучин", "Токмак", "Тольятти", "Томари", "Томмот", "Томск", "Топки", "Торез", "Торжок", "Торопец", "Тосно", "Тотьма", "Троицк", "Трубчевск", "Трёхгорный", "Туапсе", "Туймазы", "Тула", "Тулун", "Туран", "Туринск", "Тутаев", "Тында", "Тырныауз", "Тюкалинск", "Тюмень", "Уварово", "Углегорск", "Угледар", "Углич", "Удачный", "Удомля", "Ужур", "Узловая", "Украинск", "Улан-Удэ", "Ульяновск", "Унеча", "Урай", "Урень", "Уржум", "Урус-Мартан", "Урюпинск", "Усинск", "Усмань", "Усолье", "Усолье-Сибирское", "Уссурийск", "Усть-Джегута", "Усть-Илимск", "Усть-Катав", "Усть-Кут", "Усть-Лабинск", "Устюжна", "Уфа", "Ухта", "Учалы", "Уяр", "Фатеж", "Феодосия", "Фокино", "Форос", "Фролово", "Фрязино", "Фурманов", "Хабаровск", "Хадыженск", "Ханты-Мансийск", "Харабали", "Харовск", "Харцызск", "Хасавюрт", "Хвалынск", "Херсон", "Хилок", "Химки", "Холм", "Холмск", "Хоста", "Хотьково", "Царское село", "Цивильск", "Цимлянск", "Циолковский", "Чадан", "Чайковский", "Чапаевск", "Чаплыгин", "Часов Яр", "Чебаркуль", "Чебоксары", "Чегем", "Чекалин", "Челябинск", "Червонопартизанск", "Чердынь", "Черемхово", "Черепаново", "Череповец", "Черкесск", "Черноголовка", "Черногорск", "Черноморское", "Чернушка", "Черняховск", "Чехов", "Чистополь", "Чита", "Чкаловск", "Чудово", "Чулым", "Чусовой", "Чухлома", "Чёрмоз", "Шагонар", "Шадринск", "Шали", "Шарыпово", "Шарья", "Шатура", "Шахты", "Шахтёрск", "Шахунья", "Шацк", "Шебекино", "Шелехов", "Шенкурск", "Шерегеш", "Шилка", "Шимановск", "Шиханы", "Шлиссельбург", "Штормовое", "Шумерля", "Шумиха", "Шуя", "Щелкино", "Щигры", "Щучье", "Щёкино", "Щёлкино", "Щёлково", "Электрогорск", "Электросталь", "Электроугли", "Элиста", "Энгельс", "Энергодар", "Эртиль", "Югорск", "Южа", "Южно-Сахалинск", "Южно-Сухокумск", "Южноуральск", "Юнокоммунаровск", "Юрга", "Юрьев-Польский", "Юрьевец", "Юрюзань", "Юхнов", "Ядрин", "Якутск", "Ялта", "Ялуторовск", "Янаул", "Яранск", "Яровое", "Ярославль", "Ярцево", "Ясиноватая", "Ясногорск", "Ясный", "Яхрома"};
    QCompleter *locationCompleter = new QCompleter(predefinedCities, this);
    locationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    locationLineEdit->setCompleter(locationCompleter);

    // Кнопка поиска
    QPushButton *searchButton = new QPushButton("Поиск", this);
    searchButton->setStyleSheet("QPushButton {font-size: 40px; background-color: #DC49B0; border: none; color: white; padding: 0 36px 0 36px;}"
                                "QPushButton:hover {background-color: #BC2990;}");
    searchButton->setFixedHeight(80);
    searchLayout->addWidget(searchButton);
    qDebug() << "Main layout margins before:" << mainLayout->contentsMargins();
    mainLayout->addLayout(searchLayout);

    // Фильтры
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setContentsMargins(0, 20, 0, 0); // Отступ сверху для фильтров
    //filterLayout->setSpacing(0);


    // Категории
    QVBoxLayout *categories = new QVBoxLayout();
    //categories->setContentsMargins(0, 0, 0, 0);


    QLabel *categoryLabel = new QLabel("Категории:", this);
    categoryLabel->setStyleSheet("font-size: 32px; margin-bottom: 4px;");
    categories->addWidget(categoryLabel);

    categoryDropdown = new QComboBox(this);
    categoryDropdown->setFixedHeight(80);
    categoryDropdown->addItem("Все категории");
    categoryDropdown->addItems(DatabaseHandler::instance()->getCategoryNames());
    categoryDropdown->setFixedSize(600,80);
    categoryDropdown->setStyleSheet("QComboBox { font-size: 32px; padding: 6px 0px 6px 80px; color: black; border: 1px solid #BCBCBC; }"
                                    "QComboBox::drop-down {border: 0px; width: 80px; subcontrol-origin: padding; subcontrol-position: left;} "
                                    "QComboBox::down-arrow { image: url(:/images/dropdown.png); width: 80px; height: 80px; border: 1px solid #BCBCBC}");
    categoryDropdown->setStyleSheet("QComboBox { font-size: 32px; padding: 6px 0px 0px 80px; color: #BCBCBC; border: 1px solid #BCBCBC; }"
                                    "QComboBox::drop-down {border: 0px; width: 80px; subcontrol-origin: padding; subcontrol-position: left;} "
                                    "QComboBox::down-arrow { image: url(:/images/dropdown.png); width: 80px; height: 80px; border: 1px solid #BCBCBC}");

    connect(categoryDropdown, &QComboBox::currentTextChanged, this, [this]()
    {
       if(categoryDropdown->currentText() == "Все категории")
       {
           categoryDropdown->setStyleSheet("QComboBox { font-size: 32px; padding: 6px 0px 6px 80px; color: #BCBCBC; border: 1px solid #BCBCBC; }"
                                           "QComboBox::drop-down {border: 0px; width: 80px; subcontrol-origin: padding; subcontrol-position: left;} "
                                           "QComboBox::down-arrow { image: url(:/images/dropdown.png); width: 80px; height: 80px; border: 1px solid #BCBCBC}");
       } else {
           categoryDropdown->setStyleSheet("QComboBox { font-size: 32px; padding: 6px 0px 6px 80px; color: black; border: 1px solid #BCBCBC; }"
                                           "QComboBox::drop-down {border: 0px; width: 80px; subcontrol-origin: padding; subcontrol-position: left;} "
                                           "QComboBox::down-arrow { image: url(:/images/dropdown.png); width: 80px; height: 80px; border: 1px solid #BCBCBC}");
       }
    });
    categories->addWidget(categoryDropdown);

    categories->setContentsMargins(0, 0, 0, 0);
    categories->setSpacing(10); // Минимальный отступ между подписью и комбобоксом



    filterLayout->addLayout(categories, 4);
    filterLayout->addStretch(1);

    // Цены
    QVBoxLayout *priceLayout = new QVBoxLayout();
    QHBoxLayout *priceSubLayout = new QHBoxLayout();


    QLabel *priceLabel = new QLabel("Цена:", this);
    priceLabel->setStyleSheet("font-size: 32px;");
    priceLayout->addWidget(priceLabel);

    QLineEdit *priceFrom = new QLineEdit(this);
    priceFrom->setPlaceholderText("От");
    priceFrom->setFixedSize(400, 80);
    priceFrom->setValidator(new QDoubleValidator(0, 100000000, 2, this));
    priceFrom->setStyleSheet("font-size: 32px; padding: 6px;  border: 1px solid #BCBCBC;");
    priceSubLayout->addWidget(priceFrom);

    QLineEdit *priceTo = new QLineEdit(this);
    priceTo->setPlaceholderText("До");
    priceTo->setFixedSize(400, 80);
    priceTo->setValidator(new QDoubleValidator(0, 100000000, 2, this));
    priceTo->setStyleSheet("font-size: 32px; padding: 6px;  border: 1px solid #BCBCBC;");
    priceSubLayout->addWidget(priceTo);

    priceLayout->addLayout(priceSubLayout);
    priceLayout->setContentsMargins(0, 0, 0, 0);
    priceLayout->setSpacing(10);

    filterLayout->addLayout(priceLayout);
    qDebug() << "Main layout margins after:" << mainLayout->contentsMargins();
    qDebug() << "Filter layout margins:" << filterLayout->contentsMargins();
    qDebug() << "FilterLayout Geometry:" << filterLayout->geometry();
    qDebug() << "Parent Widget Geometry:" << this->geometry();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addLayout(filterLayout);
    qDebug() << "FilterLayout Geometry after:" << filterLayout->geometry();
    qDebug() << "Parent Widget Geometry:" << this->geometry();

    // Список найденных объявлений
    resultCountLabel = new QLabel("Найдено 0 объявлений:", this);
    resultCountLabel->setStyleSheet("font-size: 46px; margin: 12px 0;");
    mainLayout->addWidget(resultCountLabel);

    resultsListWidget = new QListWidget(this);
    resultsListWidget->setStyleSheet("background-color: transparent; border: none;");
    resultsListWidget->setSpacing(15);
    mainLayout->addWidget(resultsListWidget, 1);

    // Сигналы и слоты
    connect(searchLineEdit, &QLineEdit::returnPressed, searchButton, &QPushButton::click);
    connect(searchButton, &QPushButton::clicked, this, [this, priceFrom, priceTo]() {
        performSearch(categoryDropdown->currentText(), priceFrom->text(), priceTo->text());
    });
}

void SearchPage::performSearch(const QString &category, const QString &priceFrom, const QString &priceTo) {
    resultsListWidget->clear();
    qDebug() << "Ищем по категории " << category << " по цене от " << priceFrom << " до " << priceTo;

    double minPrice = priceFrom.isEmpty() ? 0 : priceFrom.toDouble();
    double maxPrice = priceTo.isEmpty() ? 1e7 : priceTo.toDouble();

    QString searchText = searchLineEdit->text();
    QString location = locationLineEdit->text();

    QList<QVariantMap> results = DatabaseHandler::instance()->searchAds(
        searchText,
        location,
        category == "Все категории" ? QString() : DatabaseHandler::instance()->getCategoryIdFromName(category),
        minPrice,
        maxPrice,
        qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"])
    );

    resultCountLabel->setText(QString("Найдено %1 объявлений:").arg(results.size()));

    for (const QVariantMap &ad : results) {
        int adId = ad["id"].toInt();
        ListingCard *listingCard = new ListingCard(adId, this);
        listingCard->setStyleSheet("background-color: white;");
        listingCard->setContentsMargins(5,15,5,15);

        QHBoxLayout *adLayout = new QHBoxLayout(listingCard);

        QWidget *underlay = new QWidget(listingCard);
        underlay->setFixedSize(300,200);
        underlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.02);"); // Полупрозрачный черный цвет
        underlay->setGeometry(0, 0, 300, 200);

        QLabel *image = new QLabel(underlay);
        QByteArray photoData = ad["photo"].toByteArray();
        if (!photoData.isEmpty()) {
            QPixmap pixmap;
            pixmap.loadFromData(photoData);
            image->setPixmap(pixmap.scaled(300, 200, Qt::KeepAspectRatio));
        } else {
            image->setFixedSize(300,200);
            image->setText("Нет фото");
        }
        image->setAlignment(Qt::AlignCenter);
        image->setFixedWidth(300);
        adLayout->addWidget(underlay);

        // Информация
        QVBoxLayout *infoLayout = new QVBoxLayout();
        QLabel *nameLabel = new QLabel(ad["name"].toString(), listingCard);
        nameLabel->setStyleSheet("font-size: 36px; font-weight: medium; margin-top: 5px;");
        infoLayout->addWidget(nameLabel);
        QLabel *sellerLabel = new QLabel(ad["username"].toString() + "  ⭐ " +  QString::number(ad["reviews"].toDouble(), 'f', 1), listingCard);
        sellerLabel->setStyleSheet("font-size: 20px; color: #333;");
        infoLayout->addWidget(sellerLabel);
        QLabel *cityLabel = new QLabel(QString("г. %1").arg(ad["geolocation"].toString()), listingCard);
        cityLabel->setStyleSheet("font-size: 18px; color: #666;");
        infoLayout->addWidget(cityLabel);
        QLabel *dateLabel = new QLabel(QDateTime::fromString(ad["date"].toString(), Qt::ISODate).toString("d MMM yyyy"), listingCard);
        dateLabel->setStyleSheet("font-size: 18px; color: #666;");
        infoLayout->addWidget(dateLabel);
        adLayout->addLayout(infoLayout);

        if (!AppContext::instance()->isAdmin()) {

            // Кнопки
            QVBoxLayout *buttonLayout = new QVBoxLayout();

            QHBoxLayout *price_n_fav_layout = new QHBoxLayout();

            QLabel *priceLabel = new QLabel(QString("%1 руб.").arg(ad["price"].toDouble(), 0, 'f', 2), listingCard);
            priceLabel->setStyleSheet("font-size: 36px; color: #DC49B0; font-weight: light;");
            price_n_fav_layout->addWidget(priceLabel);

            // Кнопка избранного
            QPushButton *favoriteButton = new QPushButton(this);
            favoriteButton->setFixedSize(75, 75); // Уменьшенная кнопка
            favoriteButton->setIconSize(QSize(70, 70));

            bool isFavorite = AppContext::instance()->isFavorite(adId);

            if (isFavorite) {
                favoriteButton->setIcon(QIcon(":/images/heart.png"));
                favoriteButton->setStyleSheet(
                    "background-color: #ffffff;"
                    "border-radius: 37px;"
                    "position: absolute;"
                    "margin: 15px;"
                );
            }
            else {
                favoriteButton->setIcon(QIcon(":/images/greyed_heart.png"));
                favoriteButton->setStyleSheet(
                    "background-color: #ffffff;"
                    "border-radius: 37px;"
                    "position: absolute;"
                    "margin: 15px;"
                );
            }

//            favoriteButton->setText(isFavorite ? "Удалить из избранного" : "Добавить в избранное");

            if (AppContext::instance()->isLoggedIn()) {
                connect(favoriteButton, &QPushButton::clicked, this, [adId, favoriteButton]() {
                    if (AppContext::instance()->isFavorite(adId)) {
                        if (DatabaseHandler::instance()->removeFavourite(
                            qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), adId)) {
                            AppContext::instance()->removeFromFavorites(adId);
                            favoriteButton->setStyleSheet(
                                "background-color: #ffffff;"
                                "border-radius: 37px;"
                                "position: absolute;"
                                "margin: 15px;"
                            );
                        }
                    } else {
                        if (DatabaseHandler::instance()->addFavourite(
                            qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), adId)) {
                            AppContext::instance()->addToFavorites(adId);
                            favoriteButton->setStyleSheet(
                                "background-color: #ffffff;"
                                "border-radius: 37px;"
                                "position: absolute;"
                                "margin: 15px;"
                            );
                        }
                    }
                });
            } else {
                connect(favoriteButton, &QPushButton::clicked, this, [this, category, priceTo, priceFrom]() {
                    LoginDialog *login = new LoginDialog();

                    qDebug() << "Проверяем состояние перед вызовом performSearch:";
                    qDebug() << "category:" << category << ", priceFrom:" << priceFrom << ", priceTo:" << priceTo;
                    qDebug() << "AppContext user:" << AppContext::instance()->getCurrentUser();
                    if(login->exec())
                        emit plshelpme();
                });
            }

            // Кнопка "Написать продавцу"
            QPushButton *msgButton = new QPushButton("Написать продавцу", listingCard);
            msgButton->setFixedHeight(100);
            msgButton->setStyleSheet("background-color: #DC49B0; font-size: 32px; padding: 6px; color: white; padding: 6px;");

            if (AppContext::instance()->isLoggedIn()) {
                connect(msgButton, &QPushButton::clicked, this, [this, ad, adId]() {
                    emit this->goToDialog(adId, AppContext::instance()->getCurrentUser()["id"].toInt(), ad["user"].toInt()); // Сигнал для начала чата с продавцом
                });
            } else {
                connect(msgButton, &QPushButton::clicked, this, [this, category, priceTo, priceFrom]() {
                    LoginDialog *login = new LoginDialog();

                    qDebug() << "Проверяем состояние перед вызовом performSearch:";
                    qDebug() << "category:" << category << ", priceFrom:" << priceFrom << ", priceTo:" << priceTo;
                    qDebug() << "AppContext user:" << AppContext::instance()->getCurrentUser();
                    if(login->exec())
                        emit plshelpme();
                });
            }

            price_n_fav_layout->addWidget(favoriteButton);
            buttonLayout->addLayout(price_n_fav_layout);
            buttonLayout->addWidget(msgButton);
            adLayout->addStretch();
            adLayout->addLayout(buttonLayout);
        }

        // Привязка карточки к QListWidget через QListWidgetItem
        QListWidgetItem *item = new QListWidgetItem(resultsListWidget);
        item->setSizeHint(listingCard->sizeHint());
        resultsListWidget->setItemWidget(item, listingCard);

        // Обработчик клика на карточке
        connect(listingCard, &ListingCard::clicked, this, [this, adId]() {
            qDebug() << "Going over to product page id: " << adId;
            emit this->goToProductPage(adId); // Сигнал для перехода на страницу продукта
        });
    }
}
