$(function () {
    if (!/Mobi/.test(navigator.userAgent)) {
        $('[data-toggle="tooltip"]').tooltip();
    }

    $('#submit').click(function () {    
        $.ajax({	// Send request
            url: "login.ard",
            method: "POST",
            dataType: "text",
            data: $('#access').serialize(),
						timeout: 10000,
						success: function (result) {						
							$('#loader').hide();
							$('#open').modal();						
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
