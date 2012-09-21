(begin
  (set! sq (lambda (x) (* x x)))
  (display (sq 2))
  
  (set! delta 
    (lambda (a b c)
      (- (sq b) (* 4 a c))))
  (display (delta 1 4 2)))
