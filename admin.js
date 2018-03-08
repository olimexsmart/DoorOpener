res = [];
$(function () {
	$('[data-toggle="tooltip"]').tooltip();
	$('#radio3')[0].checked = true;
	// Modifing the visibility of the UI
	// elements depending on the desired action
	$('#radio1').on('change', function () {
		$('.adminname').show();
		$('.adminpassword').show();
		$('.username').hide();
		$('.userpassword').hide();
		res = "lock.ard";
	});
	$('#radio1bis').on('change', function () {
		$('.adminname').show();
		$('.adminpassword').show();
		$('.username').hide();
		$('.userpassword').hide();
		res = "revokeAll.ard";
	});			
	$('#radio2').on('change', function () {
		$('.adminname').hide();
		$('.adminpassword').hide();
		$('.username').show();
		$('.userpassword').show();
		res = "revoke.ard"
	});
	$('#radio3').on('change', function () {
		$('.adminname').show();
		$('.adminpassword').show();
		$('.username').show();
		$('.userpassword').show();
		res = "add.ard";
	});
	$('#radio4').on('change', function () {
		$('.adminname').hide();
		$('.adminpassword').hide();
		$('.username').show();
		$('.userpassword').show();
		res = "check.ard";
	});
	
	
	//handle = null;
	$('#submit').click(function () {
		$.ajax({
			url: res,
			method: "POST",
			dataType: "text",
			data: $('#newsentence').find('input:visible').serialize(),
			timeout: 3000,
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