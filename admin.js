res = [];
$(function () {
	if (!/Mobi/.test(navigator.userAgent)) {
		$('[data-toggle="tooltip"]').tooltip();
	}
	// Without these two lines any options has the property checked to true
	$('#r3')[0].checked = true;
	$('#v1')[0].checked = true;
	res = "add.ard";
	// Modifing the visibility of the UI
	// elements depending on the desired action
	$('#r1').on('change', function () {
		$('.adminpassword').show(); 
		$('.userpassword').hide();
		$('#timeval').hide();
		res = "lock.ard";
	});
	$('#r2').on('change', function () {
		$('.adminpassword').show();
		$('.userpassword').hide();
		$('#timeval').hide();
		res = "revokeAll.ard";
	});			
	$('#r3').on('change', function () {
		$('.adminpassword').show();
		$('.userpassword').show();
		$('#timeval').show();
		res = "add.ard";
	});
	$('#r4').on('change', function () {
		$('.adminpassword').hide();
		$('.userpassword').show();
		$('#timeval').hide();
		res = "check.ard";
	});
	
	
	//handle = null;
	$('#submit').click(function () {
		d = $('#access').find('input:visible').serialize();
		if($('#r3')[0].checked)		
			d += '@' + (Math.floor(Date.now() / 1000) + parseInt($("input[name=v]:checked").val()));

		$.ajax({	// Send request
			url: res,
			method: "POST",
			dataType: "text",
			data: d,
			timeout: 10000,
			success: function (result) {				
				$('#loader').hide();
				$('#completed').modal();						
			},
			error: function (result) {
				$('#loader').hide();
				
				switch (result.status) {
					case 423:
						$('#locked').modal();
						break;
					case 403:
						$('#forbidden').modal();
						break;
					case 429:
						$('#tooMany').modal();
						break;
					default:
						$('#default').modal();
						break;
				}												
			}
		});
		//alert("Sent");                
		$('#loader').show();
	});
})
