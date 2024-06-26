set ns [new Simulator]

set nf [open second.nam w]
$ns namtrace-all $nf
    
set tf [open second.tr w]
$ns trace-all $tf

proc finish {} {
global ns nf tf
$ns flush-trace
close $nf
close $tf
exec nam second.nam &
exit 0
}

set n0 [$ns node]
$n0 label "A"
set n1 [$ns node]
$n1 label "B"
set n2 [$ns node]
$n2 label "C"
set n3 [$ns node]
$n3 label "D"

$ns duplex-link $n0 $n1 10Mb 2ms DropTail
$ns duplex-link $n2 $n1 10Mb 2ms DropTail
$ns duplex-link $n1 $n3 10Mb 2ms DropTail

# FTP connection between A to B.
set tcp0 [new Agent/TCP]
$ns attach-agent $n0 $tcp0
set sink0 [new Agent/TCPSink]
$ns attach-agent $n1 $sink0
set ftp0 [new Application/FTP]
$ftp0 attach-agent $tcp0

# Unreliable connection between C to B.
set udp0 [new Agent/UDP]
$ns attach-agent $n2 $udp0
set null0 [new Agent/Null]
$ns attach-agent $n1 $null0
set cbr0 [new Application/Traffic/CBR]
$cbr0 attach-agent $udp0

# Reliable connection between B to D.
set tcp1 [new Agent/TCP]
$ns attach-agent $n1 $tcp1
set sink1 [new Agent/TCPSink]
$ns attach-agent $n3 $sink1
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1

$ns connect $udp0 $null0
$ns connect $tcp0 $sink0
$ns connect $tcp1 $sink1

$ns at 0.5 "$cbr0 start"
$ns at 1 "$ftp0 start"
$ns at 1.5 "$ftp0 stop"
$ns at 2.0 "$ftp1 start"
$ns at 3.0 "finish"
$ns run