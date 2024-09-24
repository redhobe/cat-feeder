class FeederInput extends HTMLElement {
  html() {
    return `<link rel="stylesheet" href="./web-components/feeder-input.css"></link><input />`
  }

  constructor() {
    super();
    const shadowRoot = this.attachShadow({ mode: "open" });
    this.shadowRoot.innerHTML = this.html();
    this.input = shadowRoot.querySelector("input");
    this.type = this.getAttribute('type');

    this.getAttributeNames().forEach((attr) => {
      this.input.setAttribute(attr, this.getAttribute(attr));
    });
  }

  connectedCallback() {
    if (this.type === 'number') {
      this.addEventListener('blur', this.onNumberInputBlur);
      this.addEventListener('keydown', this.onNumberInputKeyPress);
    }

    if (this.type === 'time') {
      this.addEventListener('blur', this.onTimeInputBlur)
    }
  }

  setShakingAnimation() {
    this.input.classList.add('input-shaking');
    setTimeout(() => this.input.classList.remove('input-shaking'), 500);
  }

  onNumberInputKeyPress(event) {
    const isDeletingKey = event.keyCode == 8 || event.keyCode == 0 || event.keyCode == 13;
    const isDigitKey = event.keyCode >= 48 && event.keyCode <= 57;

    if (isDigitKey || isDeletingKey) {
      return true;
    }
    event.preventDefault();
    return false;
  }

  onNumberInputBlur() {
    const min = Number(this.input.min);
    const max = Number(this.input.max);
    const value = Number(this.input.value);

    if (value < min || value > max) {
      this.input.value = (value < min ? min : max).toString();
      this.setShakingAnimation();
    } else {
      this.input.value = value.toString();
    }
  }

  onTimeInputBlur() {
    if (!this.input.value) {
      this.input.value = '12:00'
      this.setShakingAnimation();
    }
  }
}

customElements.define("feeder-input", FeederInput);
