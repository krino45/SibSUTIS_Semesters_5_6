; 9 brigada
;1
(print (caddar(cdadar '((1 (2 (3 4 *)))))))
;2
; cons - берет 2 аргумета, 1 - атом (в данном случае, без знака QUOTE' - результат функции + по аргументам 1 2, т.е
; 3), 2 аргумент - лист (есть знак QUOTE ', + - не фукция, а часть листа)
(print (cons (+ 1 2) '(+ 4 6)))
; 3 ((((1)) 2) 3) 
(print (cons (cons (cons (cons 1 nil) nil) (cons 2 nil)) (cons 3 nil)))
(print (list (list (list(list 1)) 2) 3))
; 4 Функция меняет местами второй и предпоследний элементы списка 
(defun func (l)
  (append (append (list (car l) (car (last(butlast l)))) (cddr(butlast(butlast l))) (cons (cadr l) (list(car(last l))))))
)
(print (func '(1 2 3 4 5 6 7 8 9 10)))