#
# Submit a series of limit explorations
#

$ntoys = (2500, 5000, 10000)
$rescale = (100, 200, 300, 400, 500, 600, 700, 800)
$useAsym = "true"

$templateJobId = 33

# Other config

$jobUri = "http://jenks-higgs.phys.washington.edu:8080/view/LLP/job/Limit-RunLimitExtrapolation/"

# Get the example job
$templateJob = Find-JenkinsJob -JobUri $jobUri -JobId $templateJobId
$p = $templateJob.Parameters

# Now, loop, as we go.
foreach ($nt in $ntoys) {
	foreach ($rescaleEvents in $rescale) {
		$p["NToys"] = "$nt"
		$p["RescaleSignal"] = "$rescaleEvents"
		$p["UseAsymFit"] = $useAsym
		Invoke-JenkinsJob -JobUri $jobUri $p
	}
}