(define <
  (lambda (n m)
    (if (or (> n m) (= n m))
        #f
        #t)))

(define >=
  (lambda (n m)
    (or (> n m) (= n m))))

(define not
  (lambda (x)
    (if (eq #f x)
        #t
        #f)))

(define <=
  (lambda (n m)
    (not (> n m))))

(define null?
  (lambda (x)
    (eq '() x)))

(define fixnum?
  (lambda (x)
    (eq 'fixnum (type-of x))))

(define boolean?
  (lambda (x) (eq 'boolean (type-of x))))

(define pair? (lambda (x) (eq 'pair (type-of x))))

(define symbol? (lambda (x) (eq 'symbol (type-of x))))

(define char? (lambda (x) (eq 'character (type-of x))))

(define char>?
  (lambda (c1 c2)
    (let ((n1 (char->code c1))
          (n2 (char->code c2)))
      (> n1 n2))))

(define zero?
  (lambda (x) (= 0 x)))

(define positive?
  (lambda (x) (> x 0)))

(define negative?
  (lambda (x) (< x 0)))

(define !=
  (lambda (x y) (not (= x y))))

(define odd?
  (lambda (x)
    (!= 0 (remainder x 2))))

(define even?
  (lambda (x) (= 0 (remainder x 2))))
