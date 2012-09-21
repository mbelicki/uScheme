(begin
  (set! fact 
    (lambda (x) 
      (if (= 1 x)
        1
        (* x (fact (- x 1))))))
  (display (fact 8)))

