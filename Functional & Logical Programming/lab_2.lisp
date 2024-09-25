; 9 brigada
; 7
(defun repeat_atom (x y)
  (cond 
  ((> y 0)(append (list x) (repeat_atom x (- y 1))))
  )
)
; 17
(defun eq-hundred (a &optional (s 0) (l a))
  (if (null a)
      (append l (list(- 100 s)))
      (eq-hundred (cdr a) (+ (car a) s) l))
)
; 27
(defun skobochki (l)
  (cond
    ((null l) nil)
    ((null (cdr l)) (list(car l)))
    (t (list (car l) (skobochki(cdr l))))
  )
)

(print (repeat_atom 'b 3))
(print (eq-hundred '(0 102 45)))
(print (skobochki '(a b c d e f g)))