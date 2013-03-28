;;;; The initial file for liutscm
;;;; Contains definitions of library functions

;;; fixnum

;;; less than
(define <
  (lambda (n m)
    (if (or (> n m) (= n m))
        #f
        #t)))
;;; greater than or equal
(define >=
  (lambda (n m)
    (or (> n m) (= n m))))
;;; less than or equal
(define <=
  (lambda (n m)
    (not (> n m))))

(define zero?
  (lambda (x) (= 0 x)))

(define odd?
  (lambda (x) (= 1 (remainder x 2))))

(define even?
  (lambda (x) (zero? (remainder x 2))))

(define positive?
  (lambda (x) (> x 0)))

(define negative?
  (lambda (x) (< x 0)))

;;; character

(define char>?
  (lambda (c1 c2)
    (let ((n1 (char->integer c1))
          (n2 (char->integer c2)))
      (> n1 n2))))

(define char<?
  (lambda (c1 c2)
    (let ((n1 (char->integer c1))
          (n2 (char->integer c2)))
      (< n1 n2))))

(define char=?
  (lambda (c1 c2)
    (= (char->integer c1) (char->integer c2))))

(define char>=?
  (lambda (c1 c2)
    (or (char>? c1 c2) (char=? c1 c2))))

(define char<=?
  (lambda (c1 c2)
    (not (char>? c1 c2))))

(define char-upper-case?
  (lambda (c)
    (let ((n (char->integer c)))
      (and (<= (char->integer #\A) n) (<= n (char->integer #\Z))))))

(define char-lower-case?
  (lambda (c)
    (let ((n (char->integer c)))
      (and (<= (char->integer #\a) n) (<= n (char->integer #\z))))))

(define char-alphabetic?
  (lambda (c)
    (or (char-upper-case? c) (char-lower-case? c))))

(define char-numeric?
  (lambda (c)
    (let ((n (char->integer c)))
      (and (<= (char->integer #\0) n) (<= n (char->integer #\9))))))

(define char-whitespace?
  (lambda (c)
    (let ((n (char->integer c)))
      (or (= (char->integer #\n) n)
          (= (char->integer #\r) n)
          (= (char->integer #\t) n)
          (= (char->integer #\ ) n)))))

;;; string

(define string<?/aux
  (lambda (s1 s2 l1 l2 p1 p2)
    (cond ((and (>= p1 l1) (< p2 l2)) #f)
          ((and (>= p1 l1) (>= p2 l2)) #f)
          ((and (< p1 l1) (< p2 l2))
           (let ((c1 (string-ref s1 p1))
                 (c2 (string-ref s2 p2)))
             (cond ((char>? c1 c2) #f)
                   ((char=? c1 c2)
                    (string<?/aux s1 s2 l1 l2 (+ p1 2) (+ p2 1)))
                   (else #t))))
          ((and (< p1 l1) (>= p2 l2)) #t)
          (else 'error))))

(define string<?
  (lambda (s1 s2)
    (string<?/aux s1 s2 (string-length s1) (string-length s2) 0 0)))

(define not
  (lambda (x)
    (if (eq? #f x)
        #t
        #f)))

(define null?
  (lambda (x)
    (eq? '() x)))

(define fixnum?
  (lambda (x)
    (eq? 'fixnum (type-of x))))

(define boolean?
  (lambda (x) (eq? 'boolean (type-of x))))

(define pair? (lambda (x) (eq? 'pair (type-of x))))

(define symbol? (lambda (x) (eq? 'symbol (type-of x))))

(define char? (lambda (x) (eq? 'character (type-of x))))

(define !=
  (lambda (x y) (not (= x y))))
