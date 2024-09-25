; 9 brigada

;helper1
(defun remove_once(s list)
 (cond
 ((null list) list )
 ((equal s (car list)) (cdr list))
 (t (cons (car list) (remove_once s (cdr list))))
 )
)
;helper2
(defun member_sp (s list)
 (cond
 ((null list) nil)
 ((equal s (car list)) t)
 (t (member_sp s (cdr list)))
 )
)
;1
(defun set_equalp (set1 set2)
 (cond
 ((and (null set1) (null set2)) t)
 ((or (null set1) (null set2)) nil)
 ((member_sp (car set1) set2) (set_equalp (cdr set1) (remove_once (car set1) set2)))
 (t nil)
 )
)
;7
(defun set_difference (set1 set2 &optional list2)
 (cond
 ((set_equalp set1 set2) nil)
 ((null set1) set2)
 ((null set2) set1)
 ((member_sp (car set1) set2) (set_difference (cdr set1) (remove_once (car set1) set2)))
 (t (append list2 (list (car set1)) (set_difference (cdr set1) set2 list2)))
 ) 
)
(print "task 1")
(print (set_equalp '(1 2 3 (2 4) 5) '(5 3 (2 4) 1 2)))
(print (set_equalp '(1 2 3 (2 5 4) 5) '(5 3 (2 4) 1 2)))
(print (set_equalp '(2 1) '(1 2)))
(print (set_equalp '(a b (c d) e) '(a (b c) d e)))
(print (set_equalp '(a b c) '(d e f)))

(print "task 7")
(print (set_difference '(1 2 3 (2 4) 5) '(5 3 (2 4) 1 2)))
(print (set_difference '(1 2 3 (2 5 4) 5) '(5 3 (2 4) 1 2)))
(print (set_difference '(2 1) '(1 2)))
(print (set_difference '(a b (c d) e) '(a (b c) d e)))

(print "task 13")
;13
(defun functional (p l)
 (cond
 ((null l) nil)
 ((funcall p (car l)) t)
 (t (functional p (cdr l)))))

(print (functional (lambda (x) (<= x 0)) '(1 -2 3)))
(print (functional (lambda (x) (<= x 0)) '(1 2 3)))

(print (functional 'symbolp '(1 2 3 a)))
(print (functional 'symbolp '(1 2 3 4)))