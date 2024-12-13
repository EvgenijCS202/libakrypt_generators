/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2020 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Файл ak_random.с                                                                               */
/*  - содержит реализацию генераторов псевдо-случайных чисел                                       */
/* ----------------------------------------------------------------------------------------------- */
 #include <libakrypt.h>

/* ----------------------------------------------------------------------------------------------- */
#ifdef AK_HAVE_TIME_H
 #include <time.h>
#endif
#ifdef AK_HAVE_UNISTD_H
 #include <unistd.h>
#endif
#ifdef AK_HAVE_FCNTL_H
 #include <fcntl.h>
#endif
#ifdef AK_HAVE_STDLIB_H
 #include <stdlib.h>
#endif

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Инициализация генератора псевдо-случайных чисел.
    Функция используется для устанавки значение полей структуры struct random в
    значения по-умолчанию. Созданный таким образом генератор не является работоспособным.

    @param rnd указатель на структуру struct random
    @return В случае успеха возвращается ak_error_ok (ноль). В случае возникновения ошибки
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_create( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  rnd->oid = NULL;
  rnd->next = NULL;
  rnd->randomize_ptr = NULL;
  rnd->random = NULL;
  rnd->free = NULL;
  memset( &rnd->data, 0, sizeof( rnd->data ));

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param rnd указатель на структуру struct random
    @return В случае успеха возвращается ak_error_ok (ноль). В случае возникновения ошибки
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_destroy( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( rnd->free != NULL ) rnd->free( rnd );
  rnd->oid = NULL;
  rnd->next = NULL;
  rnd->randomize_ptr = NULL;
  rnd->random = NULL;
  memset( &rnd->data, 0, sizeof( rnd->data ));

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция очищает все внутренние поля, уничтожает генератор псевдо-случайных чисел
    (структуру struct random) и присваивает указателю значение NULL.

    @param rnd указатель на структуру struct random.
    @return В случае успеха возвращается ak_error_ok (ноль). В случае возникновения ошибки
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 ak_pointer ak_random_delete( ak_pointer rnd )
{
  if( rnd != NULL ) {
   ak_random_destroy(( ak_random ) rnd );
   free( rnd );
  } else ak_error_message( ak_error_null_pointer, __func__ ,
                                            "use a null pointer to a random generator" );
  return NULL;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Установка происходит путем вызова функции-члена класса random,
    которая и реализует механизм инициализации.

    @param rnd контекст генератора псевдо-случайных чисел.
    @param in указатель на данные, с помощью которых инициализируется генератор.
    @param size размер данных, в байтах.

    @return В случае успеха функция возвращает \ref ak_error_ok. В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_randomize( ak_random rnd, const ak_pointer in, const ssize_t size )
{
 if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                        "use a null pointer to random generator" );
 if( in == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                           "use a null pointer to initializator" );
 if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__,
                                                              "using a buffer with wrong length" );
 if( rnd->randomize_ptr == NULL ) return ak_error_message( ak_error_undefined_function, __func__,
                                           "randomize() function not defined for this generator" );
 return rnd->randomize_ptr( rnd, in, size );
}

/* ----------------------------------------------------------------------------------------------- */
/*! Выработка последовательности псведо-случайных данных происходит путем
    вызова функции-члена класса random.

    @param rnd контекст генератора псевдо-случайных чисел.
    @param out указатель на область памяти, в которую помещаются псевдо-случайные данные.
    @param size размер помещаемых данных, в байтах.

    @return В случае успеха функция возвращает \ref ak_error_ok. В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_ptr( ak_random rnd, const ak_pointer out, const ssize_t size )
{
 if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                        "use a null pointer to random generator" );
 if( out == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                             "use a null pointer to output data" );
 if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__,
                                                              "using a buffer with wrong length" );
 if( rnd->random == NULL ) return ak_error_message( ak_error_undefined_function, __func__,
                                                "this generator has undefined random() function" );
 return rnd->random( rnd, out, size );
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param rnd указатель на контекст генератора псевдо-случайных чисел
    @param oid OID генератора.

    @return В случае успеха возвращается ak_error_ok (ноль). В случае возникновения ошибки
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_oid( ak_random rnd, ak_oid oid )
{
  int error = ak_error_ok;

 /* выполняем проверку */
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                             "using null pointer to random generator context" );
  if( oid == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                 "using null pointer to random generator OID" );
 /* проверяем, что OID от того, что нужно */
  if( oid->engine != random_generator )
    return ak_error_message( ak_error_oid_engine, __func__ , "using oid with wrong engine" );
 /* проверяем, что OID от алгоритма, а не от каких-то там параметров  */
  if( oid->mode != algorithm )
    return ak_error_message( ak_error_oid_mode, __func__ , "using oid with wrong mode" );
 /* проверяем, что производящая функция определена */
  if( oid->func.first.create == NULL )
    return ak_error_message( ak_error_undefined_function, __func__ ,
                                                       "using oid with undefined constructor" );
 /* инициализируем контекст */
  if(( error = (( ak_function_random * )oid->func.first.create )( rnd )) != ak_error_ok )
      return ak_error_message( error, __func__, "invalid creation of random generator context");

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
  static ak_uint64 shift_value = 0; // Внутренняя статическая переменная (счетчик вызовов)

/* ----------------------------------------------------------------------------------------------- */
/*! Функция использует для генерации случайного значения текущее время, номер процесса и
    номер пользователя.
    Несмотря на кажущуюся случайность вырабатываемого значения, функция не должна использоваться для
    генерации значений, для которых требуется криптографическая случайность. Это связано с
    достаточно прогнозируемым изменением значений функции при многократных повторных вызовах.

    Основная задача данной функции - инициализация программного генератора
    каким-либо значением, в случае, когда пользователь не инициализирует программный генератор
    самостоятельно.

   \return Функция возвращает случайное число размером 8 байт (64 бита).                           */
/* ----------------------------------------------------------------------------------------------- */
 ak_uint64 ak_random_value( void )
{
  ak_uint64 vtme = 17, clk = 23, value = 1;
#ifndef _WIN32
  ak_uint64 pval = ( ak_uint64 ) getpid();
  ak_uint64 uval = ( ak_uint64 ) getuid();
#else
  ak_uint64 pval = _getpid();
  ak_uint64 uval = 67;
#endif

#ifdef AK_HAVE_TIME_H
  vtme = ( ak_uint64) time( NULL );
  clk = ( ak_uint64 ) clock();
#endif

  value = ( shift_value += 11 )*125643267795740073ULL + pval;
  value = ( value * 506098983240188723ULL ) + 71331*uval + vtme;
 return value ^ clk;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                 реализация класса rng_lcg                                       */
/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_lcg_next( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  rnd->data.val *= 125643267795740073ULL;
  rnd->data.val += 506098983240188723ULL;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_lcg_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ssize_t idx = 0;
  ak_uint8 *value = ptr;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );
 /* сначала начальное значение, потом цикл по всем элементам массива */
  rnd->data.val = value[idx];
  do {
        rnd->next( rnd );
        rnd->data.val += value[idx];
  } while( ++idx < size );

 return rnd->next( rnd );
}

/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_lcg_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ssize_t idx = 0;
  ak_uint8 *value = ptr;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                    "use a null pointer to data" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                           "use a data vector with wrong length" );
  lab_start:
    value[idx] = (ak_uint8) ( rnd->data.val >> 16 );
    rnd->next( rnd );
    if( ++idx < size ) goto lab_start;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Генератор вырабатывает последовательность внутренних состояний, удовлетворяющую
    линейному сравнению \f$ x_{n+1} \equiv a\cdot x_n + c \pmod{2^{64}}, \f$
    в котором константы a и c удовлетворяют равенствам
    \f$ a = 125643267795740073 \f$ и \f$ b = 506098983240188723. \f$

    Далее, последовательность внутренних состояний преобразуется в последовательность
    байт по следующему правилу
    \f$ \gamma_n = \displaystyle\frac{x_n - \hat x_n}{2^{24}} \pmod{256}, \f$
    где \f$\hat x_n \equiv x_n \pmod{2^{24}}. \f$

    @param generator Контекст создаваемого генератора.
    \return В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.                                                               */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_lcg( ak_random generator )
{
  int error = ak_error_ok;
  ak_uint64 qword = ak_random_value(); /* вырабатываем случайное число */

  if(( error = ak_random_create( generator )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

  generator->oid = ak_oid_find_by_name("lcg");
  generator->next = ak_random_lcg_next;
  generator->randomize_ptr = ak_random_lcg_randomize_ptr;
  generator->random = ak_random_lcg_random;

 /* для корректной работы присваиваем какое-то случайное начальное значение */
  ak_random_lcg_randomize_ptr( generator, &qword, sizeof( ak_uint64 ));
 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                 реализация класса rng_file                                      */
/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_file_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
    ak_uint8 *value = ptr;
    ssize_t result = 0, offset = 0, count = size;

    if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
    if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                   "use a null pointer to data" );
    if( size <= 0 ) return ak_error_message( ak_error_zero_length, __func__ ,
                                                                 "use a data with wrong length" );
   /* обнуляем */
    memset( ptr, 0, size );

slabel:
   /* считываем, сколько получится */
    result = read( rnd->data.fd, value +offset,
                #ifdef _MSC_VER
                  (unsigned int)
                #else
                  (size_t)
                #endif
                  count
                 );

    if( result < 0 ) { /* здесь ошибка чтения:
                          - файл мог потеряться,
                          - или устройство могло выключиться */
      return ak_error_message( ak_error_read_data, __func__, "incorrect data reading");
    }
    if( result == 0 ) {
      lseek( rnd->data.fd, 0, SEEK_SET );
      goto slabel;
    }

   /* ищменяем значения индексов */
    offset += result;
    count -= result;

    if( offset >= size ) return ak_error_ok;
    goto slabel;
}

/* ----------------------------------------------------------------------------------------------- */
 int ak_random_file_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if( close( rnd->data.fd ) == -1 )
    ak_error_message( ak_error_close_file, __func__ , "wrong closing a file with random data" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Данный генератор связывается с заданным файлом и возвращает содержащиеся в нем значения
    в качестве случайных чисел. Если данные в файле заканчиваются, то считывание начинается
    с начала файла.

    Основное назначение данного генератора - считывание данных из файловых устройств,
    таких как /dev/randon или /dev/urandom.

    @param generator Контекст создаваемого генератора.
    @param filename Имя файла.
    \return В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.                                                               */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_file( ak_random rnd, const char *filename )
{
  int error = ak_error_ok;
  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* теперь мы открываем заданный пользователем файл */
  if(( rnd->data.fd = open( filename, O_RDONLY | O_BINARY )) == -1 ) {
    ak_error_message_fmt( ak_error_open_file, __func__ ,
                                  "wrong opening a file \"%s\" with random data", filename );
    ak_random_destroy( rnd );
    return ak_error_open_file;
  }

  // для данного генератора oid не определен
  rnd->next = NULL;
  rnd->randomize_ptr = NULL;
  rnd->random = ak_random_file_ptr;
  rnd->free = ak_random_file_free;

 return error;
}

#if defined(__unix__) || defined(__APPLE__)
/* ----------------------------------------------------------------------------------------------- */
/*! @param generator Контекст создаваемого генератора.
    \return В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.                                                               */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_random( ak_random generator )
{
 int result = ak_random_create_file( generator, "/dev/random" );
  if( result == ak_error_ok ) generator->oid = ak_oid_find_by_name("dev-random");
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param generator Контекст создаваемого генератора.
    \return В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.                                                               */
/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_urandom( ak_random generator )
{
 int result = ak_random_create_file( generator, "/dev/urandom" );
  if( result == ak_error_ok ) generator->oid = ak_oid_find_by_name("dev-urandom");
 return result;
}
#endif


/* ----------------------------------------------------------------------------------------------- */
/*                                 реализация класса rng_winrtl                                    */
/* ----------------------------------------------------------------------------------------------- */
#ifdef _WIN32
 static int ak_random_winrtl_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                    "use a null pointer to data" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                           "use a data vector with wrong length" );

  if( !CryptGenRandom( rnd->data.handle, (DWORD) size, ptr ))
    return ak_error_message( ak_error_undefined_value, __func__,
                                                    "wrong generation of pseudo random sequence" );
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
 static int ak_random_winrtl_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                "freeing a null pointer to data" );
  if( !CryptReleaseContext( rnd->data.handle, 0 )) {
    return ak_error_message_fmt( ak_error_close_file,
            __func__ , "wrong closing a system crypto provider with error: %x", GetLastError( ));
  }

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
 int ak_random_create_winrtl( ak_random generator )
{
  HCRYPTPROV handle = 0;

  int error = ak_error_ok;
  if(( error = ak_random_create( generator )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

  /* теперь мы открываем криптопровайдер для доступа к генерации случайных значений
     в начале мы пытаемся создать новый ключ */
  if( !CryptAcquireContext( &handle, NULL, NULL,
                                         PROV_RSA_FULL, CRYPT_NEWKEYSET )) {
    /* здесь нам не удалось создать ключ, поэтому мы пытаемся его открыть */
    if( GetLastError() == NTE_EXISTS ) {
      if( !CryptAcquireContext( &handle, NULL, NULL,
                                            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT )) {
        ak_error_message_fmt( error = ak_error_open_file, __func__,
              "wrong open default key for system crypto provider with error: %x", GetLastError( ));
       ak_random_destroy( generator );
       return error;
      }
    } else {
       ak_error_message_fmt( error = ak_error_access_file, __func__,
                    "wrong creation of default key for system crypto provider with error: %x",
                                                                                  GetLastError( ));
       ak_random_destroy( generator );
       return error;
     }
  }

  generator->data.handle = handle;
  generator->oid = ak_oid_find_by_name("winrtl");
  generator->next = NULL;
  generator->randomize_ptr = NULL;
  generator->random = ak_random_winrtl_random;

 /* эта функция должна закрыть открытый ранее криптопровайдер */
  generator->free = ak_random_winrtl_free;
 return error;
}
#endif

/* ----------------------------------------------------------------------------------------------- */
/*! Функция заполняет заданную область памяти случайными данными, выработанными заданным
    генератором псевдослучайных чисел. Генератор должен быть предварительно корректно
    инициализирован с помощью функции вида `ak_random_create_...()`.

    @param ptr Область данных, которая заполняется случайным мусором.
    @param size Размер заполняемой области в байтах.
    @param generator Генератор псевдо-случайных чисел, используемый для генерации случайного мусора.
    @param readflag Булева переменная, отвечающая за обязательное чтение сгенерированных данных.
    В большинстве случаев должна принимать истинное значение.
    @return Функция возвращает \ref ak_error_ok (ноль) в случае успешного уничтожения данных.
    В противном случае возвращается код ошибки.                                                    */
/* ----------------------------------------------------------------------------------------------- */
 int ak_ptr_wipe( ak_pointer ptr, size_t size, ak_random rnd )
{
  size_t idx = 0;
  int error = ak_error_ok;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                "using null pointer to random generator context" );
  if( rnd->random == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                  "using uninitialized random generator context" );
  if( size > (((size_t)-1) >> 1 )) return ak_error_message( ak_error_wrong_length, __func__,
                                                                   "using very large size value" );
  if(( ptr == NULL ) || ( size == 0 )) return ak_error_ok;

  if( rnd->random( rnd, ptr, (ssize_t) size ) != ak_error_ok ) {
    memset( ptr, 0, size );
    ak_error_message( error = ak_error_write_data, __func__, "incorrect memory wiping" );
  }
 /* запись в память при чтении => необходим вызов функции чтения данных из ptr */
  for( idx = 0; idx < size; idx++ ) ((ak_uint8 *)ptr)[idx] += ((ak_uint8 *)ptr)[size - 1 - idx];
 return error;
}

/* ----------------------------------------------------------------------------------------------- */
 int ak_file_delete( const char *file, ak_random rnd )
{
  if( file == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                               "using null pointer to file name" );
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                "using null pointer to random generator context" );
  if( rnd->random == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                  "using uninitialized random generator context" );
 /* пропущен фрагмент с очищением кармы */
  #ifdef AK_HAVE_SYSMMAN_H
    /* здесь надо немного поработать ))) */
  #endif

  #ifdef AK_HAVE_UNISTD_H
   unlink( file );
  #else
   remove( file );
  #endif

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*                           реализация одиночного nlfsr генератора                                */
/* ----------------------------------------------------------------------------------------------- */
/** @brief Класс с параметрами для NLFSR генератора с квадратичной обратной связью */
 typedef struct nlfsr_register{
     /** @brief Внутреннее состояние генератора. */
     ak_uint32 state;
     /** @brief Цифровой код линейной функции обратной связи. */
     ak_uint32 linear_part;
     /** @brief Представление нелинейной функции обратной связи специального вида. */
     ak_uint32 nonlinear_part;
     /** @brief Длина регистра в данном генераторе. n<=32. */
     size_t n;
 } nlfsr_register;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Инициализирует цифровое представление нелинейной функции обратной связи для NLFSR генератора специального
 * вида.
 * @param rnd NLFSR генератор.
 */
 static void ak_initialize_nonlinear_part(ak_random rnd)
{
    unsigned non_linear = 1;
    for (size_t i = 0; i < ((nlfsr_register*)(rnd->data.ctx))->n - 3; i++)
        non_linear = (non_linear << 1) ^ non_linear;

    ((nlfsr_register*)(rnd->data.ctx))->nonlinear_part = non_linear;
}

/* ----------------------------------------------------------------------------------------------- */
 /**
  * @brief  Инициализирует NLFSR генератор специального вида переданными параметрами.
  *
  * @param rnd NLFSR генератор.
  * @param ptr Указатель на данные для инициализации NLFSR генератора.
  * @param size Количество параметров для инициализации.
  * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
  */
 static int ak_random_nlfsr_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  nlfsr_register *ctx = NULL;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

 /* текущая реализация ограничена 32-мя битами, поэтому больше 4х октетов не копируем */
  ctx = rnd->data.ctx;
  ctx->state = ((ak_uint8 *)ptr)[0];
  if( size > 1 ) { ctx->state <<= 8; ctx->state += ((ak_uint8 *)ptr)[1]; }
  if( size > 2 ) { ctx->state <<= 8; ctx->state += ((ak_uint8 *)ptr)[2]; }
  if( size > 3 ) { ctx->state <<= 8; ctx->state += ((ak_uint8 *)ptr)[3]; }

  ((nlfsr_register*)(rnd->data.ctx))->state <<= 64-((nlfsr_register*)(rnd->data.ctx))->n;
  ((nlfsr_register*)(rnd->data.ctx))->state >>= 64-((nlfsr_register*)(rnd->data.ctx))->n;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисления веса Хэмминга
 *
 * @param number Число для рассчёта веса.
 * @return size_t Как бы вес.
 */
 static inline size_t ak_hamming_weight( ak_uint32 number )
{
    number = number - ((number >> 1) & 0x55555555);
    number = (number & 0x33333333) + ((number >> 2) & 0x33333333);
    number = (number + (number >> 4)) & 0x0F0F0F0F;
    return (( number * 0x01010101 ) >> 24 ) &1;
}

/* ----------------------------------------------------------------------------------------------- */
 #define ak_random_nlfsr_next_bit2 do{ \
    state = ctx->state >> 1; \
    linear = ctx->linear_part & state; \
    non_linear = linear^( ctx->nonlinear_part & ( state ) & ( state >> 1 )); \
    hweight = ak_hamming_weight( non_linear ); \
    new_bit = lsb ^ hweight; \
    moved = new_bit << n1; \
    ctx->state = state ^ moved; \
    lsb = ctx->state & 1; \
  } while( 0 );

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Генерирует новый байт с помощью NLFSR генератора специального вида.
 * @param rnd NLFSR генератор.
 * @return char Новый байт.
 */
 static inline ak_uint8 ak_random_nlfsr_next_byte( ak_random rnd )
{
  ak_uint8 byte;
  nlfsr_register *ctx = rnd->data.ctx;
  ak_uint32 lsb = ctx->state & 1;
  ak_uint32 n1 = ctx->n -1;
  ak_uint32 state, linear, non_linear, hweight, new_bit, moved;

  ak_random_nlfsr_next_bit2; byte = lsb;

  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;
  byte <<= 1; ak_random_nlfsr_next_bit2; byte |= lsb;

  return byte;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция выработки последователности псевдо-случайных байт NLFSR генератором.
 *
 * @param rnd NLFSR генератор.
 * @param buffer Указатель на вырабатываемую последовательность.
 * @param size Размер после5довательности в байтах.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_nlfsr_random( ak_random rnd, const ak_pointer buffer, ssize_t size )
{
  ak_uint8 *value = buffer;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( buffer == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                    "use a null pointer to data" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                           "use a data vector with wrong length" );
  while( size-- > 0 )
     *value++ = ak_random_nlfsr_next_byte( rnd );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция освобождения внутреннего состояния NLFSR генератора.
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_nlfsr_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if( rnd->data.ctx != NULL)
    free(rnd->data.ctx);

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param generator NLFSR генератор
   @param size размер нелинейного регистра, не должен быть более 32-х
   @param linear_part код линейной части обратной связи
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.                                                              */
 int ak_random_create_nlfsr_with_params( ak_random generator, size_t size, ak_uint64 linear_part )
{
  int error = ak_error_ok;

  if(( error = ak_random_create( generator )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

  generator->oid = ak_oid_find_by_name("nlfsr");
  generator->next = NULL; // внутреннее состояние изменяется в функции ak_random_nlfsr_next_bit
  generator->randomize_ptr = ak_random_nlfsr_randomize_ptr;
  generator->random = ak_random_nlfsr_random;
  generator->free = ak_random_nlfsr_free;

  generator->data.ctx = ( struct nlfsr_generator* ) malloc(sizeof( nlfsr_register ));
  ((nlfsr_register*)(generator->data.ctx))->n = size;
  ((nlfsr_register*)(generator->data.ctx))->state = ak_random_value();
  ((nlfsr_register*)(generator->data.ctx))->state <<= 64-size;
  ((nlfsr_register*)(generator->data.ctx))->state >>= 64-size;
  ((nlfsr_register*)(generator->data.ctx))->linear_part = linear_part;
  ak_initialize_nonlinear_part(generator);

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param generator NLFSR генератор.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 int ak_random_create_nlfsr( ak_random generator )
{
  /*  для последовательностей с которотким циклом
   *  можно использовать следующие параметры инициализации генератора
   *    return ak_random_create_nlfsr_with_params( generator, 21, 849314 ); */
 return ak_random_create_nlfsr_with_params( generator, 32, 183599831 );
}

/* ----------------------------------------------------------------------------------------------- */
/*                                   реализация класса hrng                                        */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Класс для хранения внутренних состояний генератора hrng */
 typedef struct random_hrng {
  /*! \brief структура используемой бесключевой функции хеширования */
   struct hash hctx;
  /*! \brief текущее внутреннее состояние генератора, включая счетчик обработанных блоков */
   ak_mpzn512 counter;
  /*! \brief массив выработанных значений */
   ak_uint8 buffer[64];
  /*! \brief текущее количество доступных для выдачи октетов */
   int capacity;
 } *ak_random_hrng;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_hrng_next( ak_random rnd )
{
  int error = ak_error_ok;
  ak_random_hrng hrng = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( hrng = (ak_random_hrng) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

  hrng->counter[7]++;
  hrng->capacity = 64;
  if(( error = ak_hash_ptr( &hrng->hctx, hrng->counter, 64, hrng->buffer, 64 )) != ak_error_ok )
    ak_error_message( error, __func__, "incorrect hashing of internal state" );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_hrng_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  int error = ak_error_ok;
  ak_random_hrng hrng = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( hrng = (ak_random_hrng) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

 /* формируем внутреннее состояние */
  if(( error = ak_hash_ptr( &hrng->hctx, ptr, size, hrng->counter, 64 )) != ak_error_ok )
    return ak_error_message( error, __func__, "incorrect hashing of input data");
 /* вычисляем новое состояние промежуточного буфера */
 return rnd->next( rnd );
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_hrng_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  int cursize = size;
  ak_uint8 *outbuf = ptr;
  ak_random_hrng hrng = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( hrng = (ak_random_hrng) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

  while( hrng->capacity < cursize ) {
     memcpy( outbuf, hrng->buffer + (64 - hrng->capacity), hrng->capacity );
     cursize -= hrng->capacity;
     outbuf += hrng->capacity;
     rnd->next( rnd );
  }

  memcpy( outbuf, hrng->buffer + (64 - hrng->capacity), cursize );
  hrng->capacity -= cursize;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_hrng_free( ak_random rnd )
{
  ak_random_hrng hrng = NULL;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( hrng = (ak_random_hrng) rnd->data.ctx ) != NULL ) {
    ak_hash_destroy( &hrng->hctx );
    memset( hrng, 0, sizeof( struct random_hrng ));
    free( hrng );
  }

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 int ak_random_create_hrng( ak_random rnd )
{
  int error = ak_error_ok;
  ak_random_hrng hrng = NULL;
  ak_uint64 value = ak_random_value();

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_hrng ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

 /* создаем контекст функции хеширования */
  hrng = (ak_random_hrng) rnd->data.ctx;
  if(( error =
          ak_hash_create_streebog512( &hrng->hctx )) != ak_error_ok )
    return ak_error_message( error, __func__ , "incorrect creation of hash function context" );

 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("hrng");
  rnd->next = ak_random_hrng_next;
  rnd->randomize_ptr = ak_random_hrng_randomize_ptr;
  rnd->random = ak_random_hrng_random;
  rnd->free = ak_random_hrng_free;

 /* инициализируем начальное состояние */
  if(( error = rnd->randomize_ptr( rnd, &value, sizeof( value ))) != ak_error_ok )
   ak_error_message( error, __func__, "incorrect initialization of internal state" );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*        группа функций проверки статистического качества двоичных последовательностей            */
/* ----------------------------------------------------------------------------------------------- */
 static const ak_uint8 ak_hamming_weight_table[256] = {
     0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
     1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
     3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
     4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
 };

/* ----------------------------------------------------------------------------------------------- */
 const ak_uint8 ak_bigramm_weight_table [256][4] =
{
 { 4, 0, 0, 0}, { 3, 1, 0, 0}, { 3, 0, 1, 0}, { 3, 0, 0, 1},
 { 3, 1, 0, 0}, { 2, 2, 0, 0}, { 2, 1, 1, 0}, { 2, 1, 0, 1},
 { 3, 0, 1, 0}, { 2, 1, 1, 0}, { 2, 0, 2, 0}, { 2, 0, 1, 1},
 { 3, 0, 0, 1}, { 2, 1, 0, 1}, { 2, 0, 1, 1}, { 2, 0, 0, 2},
 { 3, 1, 0, 0}, { 2, 2, 0, 0}, { 2, 1, 1, 0}, { 2, 1, 0, 1},
 { 2, 2, 0, 0}, { 1, 3, 0, 0}, { 1, 2, 1, 0}, { 1, 2, 0, 1},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 3, 0, 1, 0}, { 2, 1, 1, 0}, { 2, 0, 2, 0}, { 2, 0, 1, 1},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 2, 0, 2, 0}, { 1, 1, 2, 0}, { 1, 0, 3, 0}, { 1, 0, 2, 1},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 3, 0, 0, 1}, { 2, 1, 0, 1}, { 2, 0, 1, 1}, { 2, 0, 0, 2},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 2, 0, 0, 2}, { 1, 1, 0, 2}, { 1, 0, 1, 2}, { 1, 0, 0, 3},
 { 3, 1, 0, 0}, { 2, 2, 0, 0}, { 2, 1, 1, 0}, { 2, 1, 0, 1},
 { 2, 2, 0, 0}, { 1, 3, 0, 0}, { 1, 2, 1, 0}, { 1, 2, 0, 1},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 2, 2, 0, 0}, { 1, 3, 0, 0}, { 1, 2, 1, 0}, { 1, 2, 0, 1},
 { 1, 3, 0, 0}, { 0, 4, 0, 0}, { 0, 3, 1, 0}, { 0, 3, 0, 1},
 { 1, 2, 1, 0}, { 0, 3, 1, 0}, { 0, 2, 2, 0}, { 0, 2, 1, 1},
 { 1, 2, 0, 1}, { 0, 3, 0, 1}, { 0, 2, 1, 1}, { 0, 2, 0, 2},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 1, 2, 1, 0}, { 0, 3, 1, 0}, { 0, 2, 2, 0}, { 0, 2, 1, 1},
 { 1, 1, 2, 0}, { 0, 2, 2, 0}, { 0, 1, 3, 0}, { 0, 1, 2, 1},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 1, 2, 0, 1}, { 0, 3, 0, 1}, { 0, 2, 1, 1}, { 0, 2, 0, 2},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 1, 1, 0, 2}, { 0, 2, 0, 2}, { 0, 1, 1, 2}, { 0, 1, 0, 3},
 { 3, 0, 1, 0}, { 2, 1, 1, 0}, { 2, 0, 2, 0}, { 2, 0, 1, 1},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 2, 0, 2, 0}, { 1, 1, 2, 0}, { 1, 0, 3, 0}, { 1, 0, 2, 1},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 2, 1, 1, 0}, { 1, 2, 1, 0}, { 1, 1, 2, 0}, { 1, 1, 1, 1},
 { 1, 2, 1, 0}, { 0, 3, 1, 0}, { 0, 2, 2, 0}, { 0, 2, 1, 1},
 { 1, 1, 2, 0}, { 0, 2, 2, 0}, { 0, 1, 3, 0}, { 0, 1, 2, 1},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 2, 0, 2, 0}, { 1, 1, 2, 0}, { 1, 0, 3, 0}, { 1, 0, 2, 1},
 { 1, 1, 2, 0}, { 0, 2, 2, 0}, { 0, 1, 3, 0}, { 0, 1, 2, 1},
 { 1, 0, 3, 0}, { 0, 1, 3, 0}, { 0, 0, 4, 0}, { 0, 0, 3, 1},
 { 1, 0, 2, 1}, { 0, 1, 2, 1}, { 0, 0, 3, 1}, { 0, 0, 2, 2},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 1, 0, 2, 1}, { 0, 1, 2, 1}, { 0, 0, 3, 1}, { 0, 0, 2, 2},
 { 1, 0, 1, 2}, { 0, 1, 1, 2}, { 0, 0, 2, 2}, { 0, 0, 1, 3},
 { 3, 0, 0, 1}, { 2, 1, 0, 1}, { 2, 0, 1, 1}, { 2, 0, 0, 2},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 2, 0, 0, 2}, { 1, 1, 0, 2}, { 1, 0, 1, 2}, { 1, 0, 0, 3},
 { 2, 1, 0, 1}, { 1, 2, 0, 1}, { 1, 1, 1, 1}, { 1, 1, 0, 2},
 { 1, 2, 0, 1}, { 0, 3, 0, 1}, { 0, 2, 1, 1}, { 0, 2, 0, 2},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 1, 1, 0, 2}, { 0, 2, 0, 2}, { 0, 1, 1, 2}, { 0, 1, 0, 3},
 { 2, 0, 1, 1}, { 1, 1, 1, 1}, { 1, 0, 2, 1}, { 1, 0, 1, 2},
 { 1, 1, 1, 1}, { 0, 2, 1, 1}, { 0, 1, 2, 1}, { 0, 1, 1, 2},
 { 1, 0, 2, 1}, { 0, 1, 2, 1}, { 0, 0, 3, 1}, { 0, 0, 2, 2},
 { 1, 0, 1, 2}, { 0, 1, 1, 2}, { 0, 0, 2, 2}, { 0, 0, 1, 3},
 { 2, 0, 0, 2}, { 1, 1, 0, 2}, { 1, 0, 1, 2}, { 1, 0, 0, 3},
 { 1, 1, 0, 2}, { 0, 2, 0, 2}, { 0, 1, 1, 2}, { 0, 1, 0, 3},
 { 1, 0, 1, 2}, { 0, 1, 1, 2}, { 0, 0, 2, 2}, { 0, 0, 1, 3},
 { 1, 0, 0, 3}, { 0, 1, 0, 3}, { 0, 0, 1, 3}, { 0, 0, 0, 4}
};

/* ----------------------------------------------------------------------------------------------- */
/*! В ходе выполнения функции выполняется проверка статистической гипотезы о равномерном
    распределении нулей и единиц в заданном массиве данных.

    @param data указатель на область данных
    @param size размер данных (в байтах), допустимые значения 32, 64 и 128.
    @return В случае, если гипотеза о равномерном распределении не отвергается,
    возвращается истина, в противном случае - ложь.                                                */
/* ----------------------------------------------------------------------------------------------- */
 bool_t ak_random_dynamic_test( ak_uint8 *data, size_t size )
{
    size_t idx = 0;
    size_t binsum = 0; /* сумма единиц */
    size_t bigrsum[4] = { 0, 0, 0, 0 }; /* суммы биграмм */
    long double chi = 0; /* значение статистички хи-квадрат для биграмм */

    for( idx = 0; idx < size; idx++ )
    {
        binsum += ak_hamming_weight_table[data[idx]];
        bigrsum[0] += ak_bigramm_weight_table[data[idx]][0];
        bigrsum[1] += ak_bigramm_weight_table[data[idx]][1];
        bigrsum[2] += ak_bigramm_weight_table[data[idx]][2];
        bigrsum[3] += ak_bigramm_weight_table[data[idx]][3];
    }

    chi += (bigrsum[0] - size )*(bigrsum[0] - size );
    chi += (bigrsum[1] - size )*(bigrsum[1] - size );
    chi += (bigrsum[2] - size )*(bigrsum[2] - size );
    chi += (bigrsum[3] - size )*(bigrsum[3] - size );
    chi /= (long double) size;

    if( ak_log_get_level() >= ak_log_maximum )
    {
        ak_error_message_fmt( ak_error_ok, __func__, "size: %u, binarySum: %u",
                                                        (unsigned int)size, (unsigned int)binsum );
        ak_error_message_fmt( ak_error_ok, __func__,
                   "s[0]: %u, s[1]: %u, s[2]: %u, s[3]: %u, chi: %.6Lf", (unsigned int)bigrsum[0],
               (unsigned int)bigrsum[1], (unsigned int)bigrsum[2], (unsigned int)bigrsum[3], chi );
    }

   /* проверяем граничные условия */
    switch( size )
    {
       case 32:
         if(( binsum < 102 ) || ( binsum > 154 )) return ak_false;
         break;
       case 64:
         if(( binsum < 219 ) || ( binsum > 293 )) return ak_false;
         break;
       case 128:
         if(( binsum < 460 ) || ( binsum > 564 )) return ak_false;
         break;
       default:
         return ak_false;
    }

    if( chi > 16.266 ) return ak_false;
  return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
/*                   реализация класса квадратичного конгруэтного метода                           */
/* ----------------------------------------------------------------------------------------------- */

/*! \brief Класс для хранения внутренних состояний генератора квадратичного конгруэтного метода */
typedef struct random_quadratic_congruence_method
{
  /*! \brief Текущее значение генератора */
  ak_uint32 xn;
  /*! \brief Коэффициент перед xn^2 */
  ak_uint32 d;
  /*! \brief Коэффициент перед xn */
  ak_uint32 a;
  /*! \brief Свободный член */
  ak_uint32 c;
  /*! \brief Модуль */
  ak_uint32 m;
} *ak_random_qcg;

const ak_uint32 random_quadratic_congruence_method_a = 0x57FF7;
const ak_uint32 random_quadratic_congruence_method_c = 0x418693;
const ak_uint32 random_quadratic_congruence_method_d = 0x2;
const ak_uint32 random_quadratic_congruence_method_m = 0xFFFFFFFC;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_qcg_next(ak_random rnd)
{

  ak_random_qcg qcg = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );

  if( (qcg = (ak_random_qcg)rnd->data.ctx) == NULL )
    return ak_error_message( ak_error_undefined_value, __func__ ,
                                                     "using non initialized context" );

  qcg->xn = (((qcg->d*((qcg->xn*qcg->xn) % qcg->m)) % qcg->m) + ((qcg->a*qcg->xn) % qcg->m) + qcg->c) % qcg->m;

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_qcg_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ak_random_qcg ctx = NULL;
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( (ak_uint64)size < sizeof(ak_uint32) ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ctx = rnd->data.ctx;
  ctx->xn = ((ak_uint32 *)ptr)[0];

  if( (ak_uint64)size < 5*sizeof(ak_uint32) ) return ak_error_ok;
  ctx->a = ((ak_uint32 *)ptr)[1];
  ctx->c = ((ak_uint32 *)ptr)[2];
  ctx->d = ((ak_uint32 *)ptr)[3];
  ctx->m = ((ak_uint32 *)ptr)[4];

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_qcg_random(ak_random rnd, const ak_pointer ptr, const ssize_t size)
{
  int error = ak_error_ok;
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ak_uint8 *outbuf = ptr;
  ssize_t i = 0;
  while (i < size)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    ak_uint8* genptr = (ak_uint8*)(&((ak_random_qcg)rnd->data.ctx)->xn)+sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
  }

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_qcg_free(ak_random rnd)
{
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");

  memset(rnd->data.ctx, 0, sizeof(struct random_quadratic_congruence_method));
  free(rnd->data.ctx);

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
int ak_random_create_qcg( ak_random rnd )
{
  int error = ak_error_ok;
  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );
 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_quadratic_congruence_method ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }
 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("qcg");
  rnd->next = ak_random_qcg_next;
  rnd->randomize_ptr = ak_random_qcg_randomize_ptr;
  rnd->random = ak_random_qcg_random;
  rnd->free = ak_random_qcg_free;
  ((ak_random_qcg)rnd->data.ctx)->a = random_quadratic_congruence_method_a;
  ((ak_random_qcg)rnd->data.ctx)->c = random_quadratic_congruence_method_c;
  ((ak_random_qcg)rnd->data.ctx)->d = random_quadratic_congruence_method_d;
  ((ak_random_qcg)rnd->data.ctx)->m = random_quadratic_congruence_method_m;

 /* инициализируем начальное состояние */
  ak_uint32 x0 = 0x8299;
  if(( error = (rnd->randomize_ptr( rnd, &x0, 4))) != ak_error_ok )
   return ak_error_message( error, __func__, "incorrect initialization of internal state" );
  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                   реализация класса квадратичного метода Ковэю                                  */
/* ----------------------------------------------------------------------------------------------- */

/*! \brief Класс для хранения внутренних состояний генератора квадратичного конгруэтного метода */
typedef struct random_quadratic_coveyou_method
{
  /*! \brief Текущее значение генератора */
  ak_uint32 x;
  /*! \brief Модуль */
  ak_uint32 m;
} *ak_random_coveyou;

const ak_uint32 random_coveyou_m = 0xFFFFFFFB;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_coveyou_next(ak_random rnd)
{

  ak_random_coveyou coveyou = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );

  if( (coveyou = (ak_random_coveyou)rnd->data.ctx) == NULL )
    return ak_error_message( ak_error_undefined_value, __func__ ,
                                                     "using non initialized context" );

  coveyou->x = ((ak_uint64)coveyou->x * (coveyou->x + 1)) % coveyou->m;

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_coveyou_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ak_random_coveyou ctx = NULL;
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( (ak_uint64)size < sizeof(ak_uint32) ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ctx = rnd->data.ctx;
  if( (((ak_uint32 *)ptr)[0] & 3) != 2 ) return ak_error_message( ak_error_invalid_value, __func__ ,
                                                          "use invalid init value" );
  ctx->x = ((ak_uint32 *)ptr)[0];

  if( (ak_uint64)size < 2*sizeof(ak_uint32) ) return ak_error_ok;
  ctx->m = ((ak_uint32 *)ptr)[1];

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_coveyou_random(ak_random rnd, const ak_pointer ptr, const ssize_t size)
{
  int error = ak_error_ok;
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ak_uint8 *outbuf = ptr;
  ssize_t i = 0;
  while (i < size)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    ak_uint8* genptr = (ak_uint8*)(&((ak_random_coveyou)rnd->data.ctx)->x)+sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
  }

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_coveyou_free(ak_random rnd)
{
  if (rnd == NULL) return ak_error_message(ak_error_null_pointer, __func__,
                                                       "use a null pointer to a random generator");

  memset(rnd->data.ctx, 0, sizeof(struct random_quadratic_coveyou_method));
  free(rnd->data.ctx);

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
int ak_random_create_coveyou(ak_random rnd )
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_quadratic_coveyou_method ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("coveyou");
  rnd->next = ak_random_coveyou_next;
  rnd->randomize_ptr = ak_random_coveyou_randomize_ptr;
  rnd->random = ak_random_coveyou_random;
  rnd->free = ak_random_coveyou_free;

 /* инициализируем начальное состояние */
  ((ak_random_coveyou)rnd->data.ctx)->m = random_coveyou_m;
  ak_uint32 x0 = 0x380316;
  if(( error = rnd->randomize_ptr( rnd, &x0, 4)) != ak_error_ok )
   return ak_error_message( error, __func__, "incorrect initialization of internal state" );
  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                   реализация класса фибоначи                                    */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Класс для хранения внутренних состояний генератора фибоначи */
 typedef struct random_fibonachi {
  /*! \brief текущее значание генератора */
  ak_uint32 xn;
  /*! \brief предыдущее значение генератора */
  ak_uint32 xprev;
  /*! \brief модуль */
  ak_uint32 m;
 } *ak_random_fibonachi;

 const ak_uint32 random_fibonachi_m = 0xFFFFFFFB;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_fibonachi_next( ak_random rnd )
{
  int error = ak_error_ok;
  ak_random_fibonachi fibonachi = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( fibonachi = (ak_random_fibonachi) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

  ak_uint32 next = ((ak_uint64)fibonachi->xn + fibonachi->xprev) % fibonachi->m;
  fibonachi->xprev = fibonachi->xn;
  fibonachi->xn = next;
 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_fibonachi_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ak_random_fibonachi ctx = NULL;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( (ak_uint64)size < 2*sizeof(ak_uint32) ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ctx = rnd->data.ctx;
  ctx->xn = ((ak_uint32 *)ptr)[0];
  ctx->xprev = ((ak_uint32 *)ptr)[1];

  if( (ak_uint64)size < 3*sizeof(ak_uint32) ) return ak_error_ok;
  ctx->m = ((ak_uint32 *)ptr)[2];

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_fibonachi_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  int error = ak_error_ok;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );
  
  ak_uint8 *outbuf = ptr;
  ssize_t i = 0;
  while (i < size)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    ak_uint8* genptr = (ak_uint8*)(&((ak_random_fibonachi)rnd->data.ctx)->xn)+sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
  }

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_fibonachi_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  memset( rnd->data.ctx, 0, sizeof( struct random_fibonachi ));
  free( rnd->data.ctx );
  
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 int ak_random_create_fibonachi( ak_random rnd )
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_fibonachi ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }
  
 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("fibonachi");
  rnd->next = ak_random_fibonachi_next;
  rnd->randomize_ptr = ak_random_fibonachi_randomize_ptr;
  rnd->random = ak_random_fibonachi_random;
  rnd->free = ak_random_fibonachi_free;

 /* инициализируем начальное состояние */
  ((ak_random_fibonachi)rnd->data.ctx)->m = random_fibonachi_m;
  ak_uint32 init[2] = { 0x4E5F6A7B, 0xA1B2C3D4 };
  if(( error = rnd->randomize_ptr( rnd, init, 8 )) != ak_error_ok )
   return ak_error_message( error, __func__, "incorrect initialization of internal state" );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                   реализация класса Грина Смита Клема                           */
/* ----------------------------------------------------------------------------------------------- */

/*! \brief Класс для хранения внутренних состояний генератора Грина Смита Клема */
typedef struct random_gck {
  /*! \brief текущее состояние */
  ak_uint32 next;
  /*! \brief количество хранимых предыдущих состояиний */
  ak_uint64 k;
  /*! \brief предыдущие k состояний */
  ak_uint32 *x;
  /*! \brief модуль */
  ak_uint32 m;
 } *ak_random_gck;

const ak_uint64 random_gck_k = 47;
const ak_uint32 random_gck_m = 0xFFFFFFFB;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_gck_next( ak_random rnd )
{
  int error = ak_error_ok;
  ak_random_gck gck = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( gck = (ak_random_gck) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

  ak_uint32 next = ((ak_uint64)gck->next + gck->x[0])%gck->m;
  ak_uint32 *buf = malloc(sizeof(ak_uint32) * (gck->k-1));
  memcpy(buf,gck->x + sizeof(ak_uint32), sizeof(ak_uint32) * (gck->k-1));
  memcpy(gck->x, buf, sizeof(ak_uint32) * (gck->k-1));
  memset (buf, 0, sizeof(ak_uint32) * (gck->k-1));
  free(buf);
  memcpy(&gck->x[gck->k-1], &gck->next, sizeof(ak_uint32));
  gck->next = next;
  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_gck_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ak_random_gck ctx = NULL;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size % 4 != 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  ctx = rnd->data.ctx;
  ctx->k = size>>2;
  ctx->x = realloc( ctx->x, (ssize_t)sizeof(ak_uint32) * (size>>2));
  memcpy( ctx->x, ptr, sizeof(ak_uint32) * (size>>2) );
  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_gck_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  int error = ak_error_ok;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );
  
  ak_uint8 *outbuf = ptr;
  ssize_t i = 0;
  while (i < size)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    ak_uint8* genptr = (ak_uint8*)(&((ak_random_gck)rnd->data.ctx)->next)+sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
  }
  
  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_gck_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  memset(((ak_random_gck)rnd->data.ctx)->x, 0, ((ak_random_gck)rnd->data.ctx)->k * sizeof(ak_uint32));
  free(((ak_random_gck)rnd->data.ctx)->x);
  memset( rnd->data.ctx, 0, sizeof( struct random_gck ));
  free( rnd->data.ctx );
  
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 int ak_random_create_gck( ak_random rnd )
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_gck ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("gck");
  rnd->next = ak_random_gck_next;
  rnd->randomize_ptr = ak_random_gck_randomize_ptr;
  rnd->random = ak_random_gck_random;
  rnd->free = ak_random_gck_free;
  ((ak_random_gck)rnd->data.ctx)->x = malloc(1);
  ((ak_random_gck)rnd->data.ctx)->m = random_gck_m;

 /* инициализируем начальное состояние */
 ak_uint32 init[45] = {
  0x74b3f95c, 0x8f6ea02e, 0x0127d3ab, 0xf30c98bd, 0x5d86a1e2,
  0xdba46cc1, 0x6e7934df, 0xca47b8f0, 0x3e91dc2a, 0x89fa4cc9,
  0x1273bf04, 0x40e95dea, 0xc1567ff1, 0x9ed3a6c8, 0x28fae80c,
  0x6db05bc2, 0x718c9f75, 0x24d6715f, 0x9b6949e8, 0xa390df17,
  0xc0831a62, 0x46fd8dc3, 0x5b4e63f9, 0x2c3ae4b7, 0x7f19d209,
  0x9d20e151, 0xeb3ca74b, 0x66a1cd0e, 0xf4be9a6d, 0x3bddf0a5,
  0x0f71b93a, 0x5c820df4, 0x9a546362, 0x32fe8cc0, 0xbbb2a4db,
  0xe8623d97, 0x151de34c, 0x8cec96ed, 0x615f7432, 0x2f40abd1,
  0x49ea6738, 0x0a2b193f, 0xe5150c29, 0x7b034c16, 0x14cf9bb1 };
 if(( error = rnd->randomize_ptr( rnd, init, 45 * sizeof(ak_uint32) )) != ak_error_ok )
  return ak_error_message( error, __func__, "incorrect initialization of internal state" );
 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                   реализация класса Митчела                                     */
/* ----------------------------------------------------------------------------------------------- */

/*! \brief Класс для хранения внутренних состояний генератора Митечела */
typedef struct random_mitchel {
  /*! \brief текущее состояние */
  ak_uint32 next;
  /*! \brief предыдущие k состояний */
  ak_uint32 *x;
  /*! \brief модуль */
  ak_uint32 m;
 } *ak_random_mitchel;

 const ak_uint32 random_mitchel_m = 0xFFFFFFFB;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_mitchel_next( ak_random rnd )
{
  int error = ak_error_ok;
  ak_random_mitchel mitchel = NULL;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  if(( mitchel = (ak_random_mitchel) rnd->data.ctx ) == NULL )
   return ak_error_message( ak_error_undefined_value, __func__, "using non initialized context" );

 ak_uint32 next = ((ak_uint64)mitchel->x[0] + mitchel->x[30]) % mitchel->m;
 ak_uint32 *buf = malloc(sizeof(ak_uint32) * 54);
 memcpy(buf,&mitchel->x[1], sizeof(ak_uint32) * 54);
 memcpy(mitchel->x, buf, sizeof(ak_uint32) * 54);
 memset (buf, 0, sizeof(ak_uint32) * 54);
 free(buf);
 memcpy(&mitchel->x[54], &mitchel->next, sizeof(ak_uint32));
 mitchel->next = next;

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, где располагаются данные,
 *            которыми инициализируется генератор псевдослучайных чисел
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_mitchel_randomize_ptr( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  ak_random_mitchel ctx = NULL;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );

  ctx = rnd->data.ctx;
  if( (ak_uint64)size < 55*sizeof(ak_uint32) ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );

  memcpy( ctx->x, ptr, sizeof(ak_uint32) * 55 );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_mitchel_random( ak_random rnd, const ak_pointer ptr, const ssize_t size )
{
  int error = ak_error_ok;
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                      "use a null pointer to a random generator" );
  if( ptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                          "use a null pointer to initial vector" );
  if( size <= 0 ) return ak_error_message( ak_error_wrong_length, __func__ ,
                                                          "use initial vector with wrong length" );
  
  ak_uint8 *outbuf = ptr;
  ssize_t i = 0;
  while (i < size)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    ak_uint8* genptr = (ak_uint8*)(&((ak_random_mitchel)rnd->data.ctx)->next)+sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
    if(i == size) return error;
    genptr += sizeof(ak_uint8);
    memcpy(outbuf, genptr, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
    ++i;
  }

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 static int ak_random_mitchel_free( ak_random rnd )
{
  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );
  memset(((ak_random_mitchel)rnd->data.ctx)->x, 0, 55 * sizeof(ak_uint32));
  free(((ak_random_mitchel)rnd->data.ctx)->x);
  memset( rnd->data.ctx, 0, sizeof( struct random_mitchel ));
  free( rnd->data.ctx );
  
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
 int ak_random_create_mitchel( ak_random rnd )
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_fibonachi ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

 /* устанавливаем обработчики событий */
  rnd->oid = ak_oid_find_by_name("mitchel");
  rnd->next = ak_random_mitchel_next;
  rnd->randomize_ptr = ak_random_mitchel_randomize_ptr;
  rnd->random = ak_random_mitchel_random;
  rnd->free = ak_random_mitchel_free;
  ((ak_random_mitchel)rnd->data.ctx)->x = malloc(55*sizeof(ak_uint32));
  ((ak_random_mitchel)rnd->data.ctx)->m = random_mitchel_m;
 
 /* инициализируем начальное состояние */
  ak_uint32 init[55] = {
    0x9f4b4f6a, 0xc1ed8d5e, 0x3bfe8d14, 0x05b8cc52, 0x6a7e2b19,
    0xd444a710, 0xf39bd3af, 0xc9a9f92e, 0x537ce20b, 0xa4f3c218,
    0xfb93acd4, 0xc52dfd21, 0x1125bf61, 0xe8a72d6c, 0x7bada3ed,
    0x628c51be, 0x05b4a3f7, 0x2e9c8f24, 0x9ba342df, 0xeca19c3a,
    0x18f8c871, 0x4df9b16b, 0x93a7d8c2, 0x7afda0b6, 0x5d4a3c48,
    0xd3b8f5c7, 0x4af35c91, 0x1e98fa6e, 0xaa67c449, 0x84bacf3f,
    0x5e4c2a79, 0x0ba87422, 0x15e72fbb, 0x4f9c3ae6, 0x305814e0,
    0x7f210a58, 0xa7f69127, 0x7d9a3f10, 0x62db4ad5, 0x8493c6f0,
    0x5325f4db, 0x7ff8d83a, 0xfcbe7c05, 0x9c2137b3, 0x143adf57,
    0xc5b42a01, 0x6d4c06ea, 0x317f21c8, 0x1fa783f1, 0x72e59a90,
    0x49f2dcbd, 0xd8ad6ec2, 0x58f6836d, 0xb0e9c3ff, 0x329b0edc };
  if(( error = rnd->randomize_ptr( rnd, init, 55*sizeof(ak_uint32))) != ak_error_ok )
   return ak_error_message( error, __func__, "incorrect initialization of internal state" );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                   реализация класса knuth_m (Алгоритм M из книги Кнута)                         */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Класс для хранения внутренних состояний генератора knuth_m */
typedef struct random_knuth_m
{
  /*! \brief Используемый генератор X */
  ak_random source_gen_x;
  /*! \brief Используемый генератор Y */
  ak_random source_gen_y;
  /*! \brief Количество элементов внутреннего состояния */
  ak_uint8 k;
  /*! \brief Внутреннее состояние */
  ak_uint8 *v;
  /*! \brief Сгенерированное значение */
  ak_uint8 next;
} *ak_random_knuth_m;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_m_next(ak_random rnd)
{
  int error = ak_error_ok;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );

  ak_uint8 next;
  if ((error = ((ak_random_knuth_m)rnd->data.ctx)->source_gen_y->random(((ak_random_knuth_m)rnd->data.ctx)->source_gen_y, &next, 1)) != ak_error_ok)
    return error;

  ak_uint8 j = (((ak_random_knuth_m)rnd->data.ctx)->k * (ak_uint32)next) >> 8;
  ((ak_random_knuth_m)rnd->data.ctx)->next = ((ak_random_knuth_m)rnd->data.ctx)->v[j];

  if ((error = ((ak_random_knuth_m)rnd->data.ctx)->source_gen_x->random(((ak_random_knuth_m)rnd->data.ctx)->source_gen_x, &next, 1)) != ak_error_ok)
    return error;

  ((ak_random_knuth_m)rnd->data.ctx)->v[j] = next;

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_m_random(ak_random rnd, const ak_pointer ptr, const ssize_t size)
{
  int error = ak_error_ok;
  ak_uint8 *outbuf = ptr;
  for (ssize_t i = 0; i < size; i++)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    memcpy(outbuf, &((ak_random_knuth_m)rnd->data.ctx)->next, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
  }

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_m_free(ak_random rnd)
{
  int error = ak_error_ok;
  if (rnd == NULL)
    return ak_error_message(ak_error_null_pointer, __func__,
                            "use a null pointer to a random generator");

  ((ak_random_knuth_m)rnd->data.ctx)->source_gen_x = NULL;
  ((ak_random_knuth_m)rnd->data.ctx)->source_gen_y = NULL;
  memset(((ak_random_knuth_m)rnd->data.ctx)->v, 0, ((ak_random_knuth_m)rnd->data.ctx)->k);
  free(((ak_random_knuth_m)rnd->data.ctx)->v);
  ((ak_random_knuth_m)rnd->data.ctx)->v = NULL;
  memset(rnd->data.ctx, 0, sizeof(struct random_knuth_m));
  free( rnd->data.ctx );
  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
int ak_random_create_knuth_m(ak_random rnd, ak_random source_gen_x, ak_random source_gen_y, ak_uint8 k)
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_knuth_m ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

 /* устанавливаем обработчики событий */
  // rnd->oid = ak_oid_find_by_name("knuth_m");
  rnd->next = ak_random_knuth_m_next;
  rnd->randomize_ptr = NULL;
  rnd->random = ak_random_knuth_m_random;
  rnd->free = ak_random_knuth_m_free;

  ((ak_random_knuth_m)rnd->data.ctx)->source_gen_x = source_gen_x;
  ((ak_random_knuth_m)rnd->data.ctx)->source_gen_y = source_gen_y;
  ((ak_random_knuth_m)rnd->data.ctx)->k = k;
  ((ak_random_knuth_m)rnd->data.ctx)->v = malloc(k);
  if(((ak_random_knuth_m)rnd->data.ctx)->v == NULL)
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  source_gen_x->random(source_gen_x, ((ak_random_knuth_m)rnd->data.ctx)->v, k);

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*                   реализация класса knuth_b (Алгоритм B из книги Кнута)                         */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Класс для хранения внутренних состояний генератора knuth_b */
typedef struct random_knuth_b
{
  /*! \brief Используемый генератор */
  ak_random source_gen;
  /*! \brief Количество элементов внутреннего состояния */
  ak_uint8 k;
  /*! \brief Внутреннее состояние */
  ak_uint8 *v;
  /*! \brief Сгенерированное значение */
  ak_uint8 next;
} *ak_random_knuth_b;

/* ----------------------------------------------------------------------------------------------- */
/**
 * @brief Функция вычисляет новое значение внутреннего состояния генератора
 *
 * @param rnd Контекст создаваемого генератора.
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_b_next(ak_random rnd)
{
  int error = ak_error_ok;

  if( rnd == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                     "use a null pointer to a random generator" );

  ak_uint8 next;
  if ((error = ((ak_random_knuth_b)rnd->data.ctx)->source_gen->random(((ak_random_knuth_b)rnd->data.ctx)->source_gen, &next, 1)) != ak_error_ok)
    return error;

  ak_uint8 j = (((ak_random_knuth_b)rnd->data.ctx)->k * (ak_uint32)next) >> 8;
  ((ak_random_knuth_b)rnd->data.ctx)->next = ((ak_random_knuth_b)rnd->data.ctx)->v[j];

  if ((error = ((ak_random_knuth_b)rnd->data.ctx)->source_gen->random(((ak_random_knuth_b)rnd->data.ctx)->source_gen, &next, 1)) != ak_error_ok)
    return error;

  ((ak_random_knuth_b)rnd->data.ctx)->v[j] = next;

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @param ptr указатель на область памяти, куда помещаются случайные данные
 * @param size размер данных (в байтах)
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_b_random(ak_random rnd, const ak_pointer ptr, const ssize_t size)
{
  int error = ak_error_ok;
  ak_uint8 *outbuf = ptr;
  for (ssize_t i = 0; i < size; i++)
  {
    if ((error = rnd->next(rnd)) != ak_error_ok)
      return error;
    memcpy(outbuf, &((ak_random_knuth_b)rnd->data.ctx)->next, sizeof(ak_uint8));
    outbuf += sizeof(ak_uint8);
  }

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
 * @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
static int ak_random_knuth_b_free(ak_random rnd)
{
  int error = ak_error_ok;
  if (rnd == NULL)
    return ak_error_message(ak_error_null_pointer, __func__,
                            "use a null pointer to a random generator");

  ((ak_random_knuth_b)rnd->data.ctx)->source_gen = NULL;
  memset(((ak_random_knuth_b)rnd->data.ctx)->v, 0, ((ak_random_knuth_b)rnd->data.ctx)->k);
  free(((ak_random_knuth_b)rnd->data.ctx)->v);
  ((ak_random_knuth_b)rnd->data.ctx)->v = NULL;
  memset(rnd->data.ctx, 0, sizeof(struct random_knuth_b));
  free(rnd->data.ctx);

  return error;
}

/* ----------------------------------------------------------------------------------------------- */
/**
 * @param rnd контекст генератора псевдослучайных чисел
   @return int В случае успеха, функция возвращает \ref ak_error_ok. В противном случае
            возвращается код ошибки.
 */
int ak_random_create_knuth_b(ak_random rnd, ak_random source_gen, ak_uint8 k)
{
  int error = ak_error_ok;

  if(( error = ak_random_create( rnd )) != ak_error_ok )
    return ak_error_message( error, __func__ , "wrong initialization of random generator" );

 /* выделяем память под внутренний контекст генератора */
  if(( rnd->data.ctx = calloc( 1, sizeof( struct random_knuth_b ))) == NULL ) {
    ak_random_destroy( rnd );
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  }

/* устанавливаем обработчики событий */
  // rnd->oid = ak_oid_find_by_name("knuth_b");
  rnd->next = ak_random_knuth_b_next;
  rnd->randomize_ptr = NULL;
  rnd->random = ak_random_knuth_b_random;
  rnd->free = ak_random_knuth_b_free;

  ((ak_random_knuth_b)rnd->data.ctx)->source_gen = source_gen;
  ((ak_random_knuth_b)rnd->data.ctx)->k = k;
  ((ak_random_knuth_b)rnd->data.ctx)->v = malloc(sizeof(ak_uint8) * k);
  if(((ak_random_knuth_b)rnd->data.ctx)->v == NULL)
    return ak_error_message( ak_error_null_pointer, __func__, "incorrect memory allocation ");
  source_gen->random(source_gen, ((ak_random_knuth_b)rnd->data.ctx)->v, k);

  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                                                                    ak_random.c  */
/* ----------------------------------------------------------------------------------------------- */
