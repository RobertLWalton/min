package require Tk
proc every {ms body} {
    eval $body
    after $ms [list every $ms $body]
}
pack [label .clock -textvar time]
every 1000 {set ::time [clock format [clock seconds] -format %H:%M:%S]} ;# RS
