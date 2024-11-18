class FeederInput extends HTMLElement {
  static formAssociated = true;
  _internals = this.attachInternals();

  html() {
    return `<link rel="stylesheet" href="./web-components/feeder-input.css"></link><input />`
  }

  constructor() {
    super();
    // const shadowRoot = this.attachShadow({ mode: "open" });
    // this.shadowRoot.innerHTML = this.html();
    // this.input = shadowRoot.querySelector("input");
    this.type = this.getAttribute('type');
    this._contents = document.createElement('input');
    // this._contents = input.cloneNode(true);

    this.getAttributeNames().forEach((attr) => {
      this._contents.setAttribute(attr, this.getAttribute(attr));
    });
  }

  connectedCallback() {
    this.appendChild(this._contents);
    this.input = this.querySelector('input');

    if (this.type === 'number') {
      this.input.addEventListener('blur', this.onNumberInputBlur.bind(this));
      this.input.addEventListener('keydown', this.onNumberInputKeyPress.bind(this));
    }

    if (this.type === 'time') {
      this.input.addEventListener('blur', this.onTimeInputBlur.bind(this))
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

  onNumberInputBlur(event) {
    const min = Number(event.target.min);
    const max = Number(event.target.max);
    const value = Number(event.target.value);

    if (value < min || value > max) {
      event.target.value = (value < min ? min : max).toString();
      this.setShakingAnimation();
    } else {
      event.target.value = value.toString();
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
