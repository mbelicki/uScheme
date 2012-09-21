(begin
  (set! flow 
    (lambda (x) 
      (begin (display x)
        (flow (+ x 1)))))
  (flow 0))

