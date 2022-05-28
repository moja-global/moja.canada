function saveFeedback(){
    const scriptURL = 'https://script.google.com/macros/s/AKfycbyaYAxpzNS-FMcGRvfvzlKhaBh4ypVthxk8IqbZlnQG4PAyhxdNx5-kvX78TOhgzruOtw/exec'
    let data = new FormData();
    const utcStr = new Date().toUTCString();
    console.log(utcStr);
    let msg = document.getElementById('message').value
    data.append('message', msg)            
    data.append('url', window.location.href)
    data.append('type', 'textFeedback')

    console.log(data)
    fetch(scriptURL, { method: 'POST', body: data})
    .then(response => {alert("Feedback saved successfully") 
                console.log(response)        
            })
    .catch(error => alert("Feedback not saved, try again"))
}

function yes(){
    alert("Glad the documentation was useful!")
}

function no(){
    alert("Thank you for the feedback, feel free to raise an issue on our Github page, we will resolve it soon :)")
}

function feedback(){
    let msg = document.getElementById('feedback').value
    alert(msg) 
}