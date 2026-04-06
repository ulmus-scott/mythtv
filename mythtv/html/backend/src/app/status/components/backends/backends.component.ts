import { Component, OnInit, Input } from '@angular/core';
import { Backend } from '../../../services/interfaces/backend.interface';
import { TranslatePipe } from '@ngx-translate/core';


@Component({
    selector: 'app-status-backends',
    templateUrl: './backends.component.html',
    styleUrls: ['./backends.component.css', '../../status.component.css'],
    imports: [TranslatePipe]
})
export class BackendsComponent implements OnInit {
  @Input() backends? : Backend[];

  constructor() { }

  ngOnInit(): void {
  }

}
