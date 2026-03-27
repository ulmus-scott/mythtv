import { Component, OnInit, Input } from '@angular/core';
import { Frontend } from "src/app/services/interfaces/frontend.interface";
import { TranslateModule } from '@ngx-translate/core';
import { NgIf, NgFor } from '@angular/common';

@Component({
    selector: 'app-status-frontends',
    templateUrl: './frontends.component.html',
    styleUrls: ['./frontends.component.css', '../../status.component.css'],
    imports: [NgIf, NgFor, TranslateModule]
})
export class FrontendsComponent implements OnInit {
  @Input() frontends? : Frontend[];

  constructor() { }

  ngOnInit(): void {
  }

}
